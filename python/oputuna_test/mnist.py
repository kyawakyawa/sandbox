import optuna
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import torch.utils
import torch.utils.data
import torchvision
import torchvision.transforms as transforms
from tqdm import tqdm


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


batch_size = 1
epochs = 1


def objective(trial: optuna.Trial):
    lr = trial.suggest_loguniform("lr", 1e-6, 1e-1)

    device = torch.device("cuda:0")

    global batch_size
    global epochs

    net = Net()

    net = net.to(device)

    transform = transforms.Compose(
        [transforms.ToTensor(), transforms.Normalize((0.5,), (0.5,))]
    )
    trainset = torchvision.datasets.MNIST(
        root="./data", train=True, download=True, transform=transform
    )
    testset = torchvision.datasets.MNIST("./data", train=False, transform=transform)

    trainloader = torch.utils.data.DataLoader(
        trainset, batch_size=batch_size, shuffle=True, num_workers=2
    )

    testloader = torch.utils.data.DataLoader(
        testset, batch_size=batch_size, shuffle=True
    )

    criterion = nn.CrossEntropyLoss()
    optimizer = optim.SGD(net.parameters(), lr=lr)

    net.train()
    for _ in tqdm(range(epochs)):
        for _, data in enumerate(trainloader, 0):
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

    net.eval()
    correct = 0
    with torch.no_grad():
        for inputs, labels in testloader:

            inputs = inputs.to(device)
            labels = labels.to(device)

            outputs = net(inputs)

            pred = outputs.argmax(1)
            correct += pred.eq(labels.view_as(pred)).sum().item()


    return -(correct / len(testloader.dataset))  # pyright: ignore


def main():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--batch_size", type=int, default=64)
    parser.add_argument("--epoch", type=int, default=10)
    args = parser.parse_args()

    global batch_size
    global epochs

    batch_size = args.batch_size
    epochs = args.epoch

    dummy_transform = transforms.Compose(
        [transforms.ToTensor(), transforms.Normalize((0.5,), (0.5,))]
    )
    dummy = torchvision.datasets.MNIST(
        root="./data", train=True, download=True, transform=dummy_transform
    )
    del dummy

    study = optuna.create_study()
    study.optimize(objective, n_trials=10)

    print(study.best_params)


if __name__ == "__main__":
    main()
