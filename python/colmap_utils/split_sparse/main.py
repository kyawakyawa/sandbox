import tyro
import os
from dataclasses import dataclass

from read_write_model import Image as CImage
from rich.progress import track
from typing import Literal
from read_write_model import read_model, write_model
from pathlib import Path
from typing import cast

ImageDict = dict[int, CImage]


@dataclass
class Config:
    input_dir: str
    output_dir: str
    num_max_images: int
    input_type: Literal["BIN", "TXT"] = "BIN"
    output_type: Literal["BIN", "TXT"] = "BIN"


def main():
    config = tyro.cli(Config)

    cameras, images, points3D = read_model(  # pyright: ignore
        Path(config.input_dir), ext=".bin" if config.input_type == "BIN" else ".txt"
    )
    images = cast(ImageDict, images)

    new_images: ImageDict = {}
    split_cnt = 0
    for image in track(images.values(), description="Correcting image_id..."):
        new_images[image.id] = CImage(
            id=image.id,
            qvec=image.qvec,
            tvec=image.tvec,
            camera_id=image.camera_id,
            name=image.name,
            xys=image.xys,
            point3D_ids=image.point3D_ids,
        )

        if len(new_images) >= config.num_max_images:
            out_name = f"{split_cnt:04d}"
            out_dir = Path(config.output_dir) / out_name
            os.makedirs(str(out_dir), exist_ok=True)

            write_model(
                cameras,
                new_images,
                points3D,
                Path(config.output_dir) / out_name,
                ".bin" if config.output_type == "BIN" else ".txt",
            )

            new_images = {}
            split_cnt += 1


if __name__ == "__main__":
    main()
