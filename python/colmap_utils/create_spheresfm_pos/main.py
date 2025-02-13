from pathlib import Path
from dataclasses import dataclass
import tyro

from read_write_model import Image as CImage
from read_write_model import read_images_binary, read_images_text, qvec2rotmat

import numpy as np
from typing import Literal, cast


@dataclass
class Config:
    images_path: str
    output_path: str

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

    lines: list[str] = []
    for image in images.values():
        name = image.name
        txyz = image.tvec
        qvec = image.qvec

        rot_mat = qvec2rotmat(qvec)
        pose_4x4 = np.eye(4)
        pose_4x4[:3, :3] = rot_mat
        pose_4x4[:3, 3] = txyz

        inv = np.linalg.inv(pose_4x4)

        txyz = inv[:3, 3]

        lines.append(f"{name} {txyz[0]} {txyz[1]} {txyz[2]} 1 0 0 0 1 0 0 0 1")

    with open(config.output_path, "w") as f:
        f.write("\n".join(lines))
