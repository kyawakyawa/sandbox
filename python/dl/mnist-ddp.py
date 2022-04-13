import torch
import torch.cuda
import torch.utils
import torch.utils.data
import torch.utils.data.distributed
import torch.distributed as dist
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.nn.parallel import DistributedDataParallel as DDP
import torchvision
import torchvision.transforms as transforms


class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.conv1 = nn.Conv2d(1, 32, 3)  # 28x28x32 -> 26x26x32
        self.conv2 = nn.Conv2d(32, 64, 3)  # 26x26x64 -> 24x24x64
        self.pool = nn.MaxPool2d(2, 2)  # 24x24x64 -> 12x12x64
        self.dropout1 = nn.Dropout2d()
        self.fc1 = nn.Linear(12 * 12 * 64, 128)
        self.dropout2 = nn.Dropout2d()
        self.fc2 = nn.Linear(128, 10)

    def forward(self, x):
        x = F.relu(self.conv1(x))
        x = self.pool(F.relu(self.conv2(x)))
        x = self.dropout1(x)
        x = x.view(-1, 12 * 12 * 64)
        x = F.relu(self.fc1(x))
        x = self.dropout2(x)
        x = self.fc2(x)
        return x


def setup_ddp(rank: int, world_size: int, device_id: int):
    import os

    master_addr = os.environ["MASTER_ADDR"]
    master_port = os.environ["MASTER_PORT"]
    init_method = f"tcp://{master_addr}:{master_port}"
    dist.init_process_group(
        backend="nccl", init_method=init_method, world_size=world_size, rank=rank
    )

    torch.manual_seed(0)
    torch.cuda.set_device(device_id)


def train(rank: int, world_size: int, npernode: int, num_epoch: int):

    if not torch.cuda.is_available():
        return

    device_id = rank % npernode

    print("Start initialization Rank ", rank)
    setup_ddp(rank, world_size, device_id)
    print("End   initialization Rank ", rank)

    net = Net()
    net.cuda()
    net.train()

    print("set up model")

    net = DDP(net, device_ids=[device_id], find_unused_parameters=False)

    print("modle with DDP")

    transform = transforms.Compose(
        [transforms.ToTensor(), transforms.Normalize((0.5,), (0.5,))]
    )
    trainset = torchvision.datasets.MNIST(
        root="./data", train=True, download=True, transform=transform
    )

    trainsampler = torch.utils.data.distributed.DistributedSampler(
        trainset, num_replicas=world_size, shuffle=True, rank=rank
    )
    trainloader = torch.utils.data.DataLoader(
        trainset, batch_size=100, num_workers=2, sampler=trainsampler
    )
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.SGD(net.parameters(), lr=0.001, momentum=0.9)

    for epoch in range(num_epoch):
        running_loss = 0
        for i, data in enumerate(trainloader, 0):
            # get the inputs; data is a list of [inputs, labels]
            inputs, labels = data
            inputs = inputs.cuda()
            labels = labels.cuda()

            # zero the parameter gradients
            optimizer.zero_grad()

            # forward + backward + optimize
            outputs = net(inputs)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()

            # print statistics
            running_loss += loss.item()
            if rank == 0 and i % 10 == 9:  # print every 10 mini-batches
                print(f"[{epoch + 1}, {i + 1:5d}] loss: {running_loss / 2000:.6f}")
                running_loss = 0.0


def main():
    import argparse

    parser = argparse.ArgumentParser()

    parser.add_argument("--epoch", type=int, default=10)
    parser.add_argument("--npernode", type=int, required=True)
    parser.add_argument("--master_addr", type=str, default="localhost")
    parser.add_argument("--master_port", type=str, default="3000")
    parser.add_argument("--nccl_ib_disable", action="store_true")

    args = parser.parse_args()
    import os

    os.environ["MASTER_ADDR"] = args.master_addr
    os.environ["MASTER_PORT"] = str(args.master_port)

    if args.nccl_ib_disable:
        os.environ["NCCL_IB_DISABLE"] = str(1)

    mpi_rank = int(os.environ["OMPI_COMM_WORLD_RANK"])
    mpi_size = int(os.environ["OMPI_COMM_WORLD_SIZE"])

    print("MPI %d/%d" % (mpi_rank, mpi_size))

    train(mpi_rank, mpi_size, args.npernode, args.epoch)


if __name__ == "__main__":
    main()
