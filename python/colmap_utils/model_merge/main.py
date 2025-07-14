import os
from dataclasses import dataclass
from pathlib import Path
from typing import Literal

import tyro
from rich.console import Console
from read_write_model import (
    Camera,
    Image,
    read_model,
    write_cameras_text,
    write_images_text,
)

console = Console()
CameraDict = dict[int, Camera]
ImageDict = dict[int, Image]


@dataclass
class Config:
    models: list[str]
    output_path: str
    single_camera: bool = True
    sort_by_name: bool = False
    conflict: Literal["skip", "error"] = "skip"
    zero_indexed: bool = False


@dataclass
class Model:
    cameras: CameraDict
    images: ImageDict


def load_models(model_names: list[str]) -> dict[str, Model]:
    models: dict[str, Model] = {}
    for model in model_names:
        exist_bin = os.path.exists(os.path.join(model, "images.bin"))
        exist_txt = os.path.exists(os.path.join(model, "images.txt"))
        assert exist_bin or exist_txt, f"images.bin or images.txt not found in {model}"

        cameras: CameraDict = {}
        images: ImageDict = {}
        if exist_bin:
            cameras, images, _ = read_model(Path(model), ext=".bin")  # pyright: ignore
        else:
            cameras, images, _ = read_model(Path(model), ext=".txt")  # pyright: ignore

        assert type(cameras) is dict, "cameras should be a dict"
        assert type(images) is dict, "images should be a dict"

        models[model] = Model(cameras=cameras, images=images)

    return models


def merge_models(
    models: dict[str, Model],
    single_camera: bool,
    sort_by_name: bool,
    zero_indexed: bool,
) -> Model:
    merged_cameras: CameraDict = {}
    merged_images: ImageDict = {}

    image_id_offset = 0
    camera_id_offset = 0
    if single_camera:
        cameras = next(iter(models.values())).cameras
        camera = next(iter(cameras.values()))
        merged_cameras[1] = Camera(
            id=1,
            model=camera.model,
            width=camera.width,
            height=camera.height,
            params=camera.params,
        )

    used_names = set()
    for model in models.values():
        cameras = model.cameras
        images = model.images

        if not single_camera:
            for camera_id, camera in cameras.items():
                new_camera_id = camera_id + camera_id_offset
                new_camera = Camera(
                    id=new_camera_id,
                    model=camera.model,
                    width=camera.width,
                    height=camera.height,
                    params=camera.params,
                )
                assert new_camera_id not in merged_cameras
                merged_cameras[new_camera_id] = new_camera

        for image_id, image in images.items():
            new_camera_id = (
                image.camera_id + camera_id_offset if not single_camera else 1
            )

            new_image_id = image_id + image_id_offset
            assert new_image_id not in merged_images

            if image.name in used_names:
                if config.conflict == "skip":
                    continue
                elif config.conflict == "error":
                    raise ValueError(f"Duplicate image name: {image.name}")
            used_names.add(image.name)

            new_image = Image(
                id=new_image_id,
                qvec=image.qvec,
                tvec=image.tvec,
                camera_id=new_camera_id,
                name=image.name,
                xys=[],
                point3D_ids=[],
            )
            merged_images[new_image_id] = new_image

        camera_id_offset += max(cameras.keys()) + (1 if zero_indexed else 0)
        image_id_offset += max(images.keys()) + (1 if zero_indexed else 0)

    if sort_by_name:
        _merged_images = list(merged_images.values())
        # nameでソート
        _merged_images.sort(key=lambda x: x.name)
        _merged_images = [
            Image(
                id=new_id + 1,
                qvec=image.qvec,
                tvec=image.tvec,
                camera_id=image.camera_id,
                name=image.name,
                xys=image.xys,
                point3D_ids=image.point3D_ids,
            )
            for new_id, image in enumerate(_merged_images)
        ]

        merged_images = {image.id: image for image in _merged_images}

    return Model(cameras=merged_cameras, images=merged_images)


def main(config: Config):
    models = load_models(config.models)
    merged_model = merge_models(
        models, config.single_camera, config.sort_by_name, config.zero_indexed
    )

    os.makedirs(config.output_path, exist_ok=True)

    write_cameras_text(
        merged_model.cameras, os.path.join(config.output_path, "cameras.txt")
    )
    write_images_text(
        merged_model.images, os.path.join(config.output_path, "images.txt")
    )

    with open(os.path.join(config.output_path, "points3D.txt"), "w") as f:
        f.write("")


if __name__ == "__main__":
    config = tyro.cli(Config)
    print(config)
    main(config)
