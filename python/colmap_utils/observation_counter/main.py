from pathlib import Path
import numpy as np
from dataclasses import dataclass
import tyro

from read_write_model import Image as CImage
from read_write_model import read_images_binary, read_images_text

from typing import Literal, cast


@dataclass
class Config:
    images_path: str

    input_type: Literal["BIN", "TXT"] = "BIN"


ImageDict = dict[int, CImage]

if __name__ == "__main__":
    config = tyro.cli(Config)

    if config.input_type == "BIN":
        images = read_images_binary(Path(config.images_path))
    elif config.input_type == "TXT":
        images = read_images_text(Path(config.images_path))
    else:
        raise ValueError("Unknown input type")

    images = cast(ImageDict, images)

    lines: list[str] = ["#F=N X Y Z R"]
    for image in images.values():
        name = image.name
        cnt = np.where(image.point3D_ids != -1, 1, 0).sum()

        print(name + ": ", cnt.item())
