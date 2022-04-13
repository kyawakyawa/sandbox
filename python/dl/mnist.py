from typing import Any

import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.cuda
import torch.utils
import torch.utils.data
import torch.optim as optim
import torchvision
import torchvision.transforms as transforms

import wandb


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


def train(
    use_wandb: bool,
    wandb_entity: str,
    wandb_project: str,
):
    device: Any = "cpu"
    if torch.cuda.is_available():
        device = "cuda:0"
    print(device)

    learn_rate = 0.001
    epochs = 30
    batch_size = 100

    transform = transforms.Compose(
        [transforms.ToTensor(), transforms.Normalize((0.5,), (0.5,))]
    )
    trainset = torchvision.datasets.MNIST(
        root="./data", train=True, download=True, transform=transform
    )

    trainloader = torch.utils.data.DataLoader(
        trainset, batch_size=batch_size, shuffle=True, num_workers=2
    )

    net = Net()
    net = net.to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.SGD(net.parameters(), lr=learn_rate, momentum=0.9)

    if use_wandb:
        import wandb

        wandb.init(project=wandb_project, entity=wandb_entity)
        wandb.config = {
            "learning_rate": learn_rate,
            "epochs": epochs,
            "batch_size": batch_size,
        }

    for epoch in range(epochs):
        running_loss = 0
        for i, data in enumerate(trainloader, 0):
            # get the inputs; data is a list of [inputs, labels]
            inputs, labels = data
            inputs = inputs.to(device)
            labels = labels.to(device)

            # zero the parameter gradients
            optimizer.zero_grad()

            # forward + backward + optimize
            outputs = net(inputs)
            loss = criterion(outputs, labels)
            loss.backward()
            optimizer.step()

            # print statistics
            running_loss += loss.item()
            if i % 30 == 29:  # print every 30 mini-batches
                print(f"[{epoch + 1}, {i + 1:5d}] loss: {running_loss / 2000:.3f}")
                running_loss = 0.0

            if use_wandb:
                import wandb

                wandb.log({"loss": loss})
                wandb.watch(net)


def main():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--use_wandb", action="store_true")
    parser.add_argument("--wandb_entity", type=str, default="user")
    parser.add_argument("--wandb_project", type=str, default="simple_mnist")

    args = parser.parse_args()

    train(args.use_wandb, args.wandb_entity, args.wandb_project)


if __name__ == "__main__":
    main()
