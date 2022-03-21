import torch


def main():
    print("PyTorch Version              : ", torch.__version__)
    print(
        "Is CUDA available?           : ", "Yes" if torch.cuda.is_available() else "No"
    )
    print("The number of CUDA devices   : ", torch.cuda.device_count())
    print("Current CUDA device's number : ", torch.cuda.current_device())
    print("CUDA device name.            : ", torch.cuda.get_device_name())


if __name__ == "__main__":
    main()
