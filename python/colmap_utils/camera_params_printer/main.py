import argparse
import os
from pathlib import Path
from typing import cast

from read_write_model import Camera, read_cameras_binary, read_cameras_text


def main():
    parser = argparse.ArgumentParser(description="colmap camera param printer")
    parser.add_argument("-i", "--input", required=True, type=str, help="input")
    args = parser.parse_args()

    input_path: str = args.input
    if (
        os.path.basename(input_path) != "cameras.txt"
        and os.path.basename(input_path) != "cameras.bin"
    ):
        if os.path.exists(os.path.join(input_path, "cameras.txt")):
            input_path = os.path.join(input_path, "cameras.txt")
        elif os.path.exists(os.path.join(input_path, "cameras.bin")):
            input_path = os.path.join(input_path, "cameras.bin")
        else:
            assert False

    cameras = (
        read_cameras_text(Path(input_path))
        if os.path.basename(input_path) == "cameras.txt"
        else read_cameras_binary(Path(input_path))
    )

    assert len(cameras) > 0
    # TODO: print warning if len(cameras) >= 2

    cameras = cast(dict[int, Camera], cameras)

    for camera in cameras.values():
        assert camera.id == 1
        single_camera = "--ImageReader.single_camera 1"
        camera_model = f"--ImageReader.camera_model {camera.model}"

        params = ""
        for i in range(len(camera.params)):
            params += str(float(camera.params[i]))
            if i < len(camera.params) - 1:
                params += ","

        camera_params = f"--ImageReader.camera_params {params}"

        print(single_camera, end=" ")
        print(camera_model, end=" ")
        print(camera_params, end=" ")

        break


if __name__ == "__main__":
    main()
