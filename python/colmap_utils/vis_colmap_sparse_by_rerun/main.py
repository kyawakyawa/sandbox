import argparse
import tyro
import os
from typing import Literal, cast
from pathlib import Path
import numpy as np

import cv2
import rerun as rr  # pip install rerun-sdk
import rerun.blueprint as rrb
from dataclasses import dataclass

from read_write_model import Camera as CCamera
from read_write_model import Image as CImage
from read_write_model import Point3D as CPoint3D
from read_write_model import read_model


@dataclass
class Config:
    sparse_path: str
    image_dir: str

    input_type: Literal["BIN", "TXT"] = "BIN"
    all_frame: bool = False
    draw_points: bool = True

    host: str = "0.0.0.0:9876"


CameraDict = dict[int, CCamera]
ImageDict = dict[int, CImage]
Point3DDict = dict[int, CPoint3D]


def visualize_all_frame(images: ImageDict):
    for image in images.values():
        name = image.name

        txyz = image.tvec
        qxyzw = image.qvec[[1, 2, 3, 0]]

        # COLMAP's camera transform is "camera from world"
        rr.log(
            "/all/camera-" + name,
            rr.Transform3D(
                translation=txyz,
                rotation=rr.Quaternion(xyzw=qxyzw),
                from_parent=True,
                axis_length=0.02,
            ),
        )
        rr.log("camera", rr.ViewCoordinates.RDF, static=True)

        width = 960
        height = 540
        rr.log(
            "/all/camera-" + name + "/image",
            rr.Pinhole(
                resolution=[width, height],
                focal_length=[width / 5, width / 5],
                principal_point=[width / 2, height / 2],
                image_plane_distance=0.02,
            ),
        )


def visualize_points3D(points3D: Point3DDict, parent=""):
    xyzs = [point3D.xyz for point3D in points3D.values()]
    rgbs = [point3D.rgb for point3D in points3D.values()]
    radii = np.ones(len(xyzs)) * 0.01

    rr.log(f"{parent}/all_points", rr.Points3D(xyzs, colors=rgbs, radii=radii))


def setup_coordinates():
    rr.log("/", rr.ViewCoordinates.RIGHT_HAND_Y_DOWN, static=True)


def visualize_images(
    cameras: CameraDict, images: ImageDict, image_dir: str = "", parent: str = ""
):
    for image in images.values():
        txyz = image.tvec
        qxyzw = image.qvec[[1, 2, 3, 0]]

        # COLMAP's camera transform is "camera from world"
        rr.log(
            f"{parent}/camera",
            rr.Transform3D(
                translation=txyz,
                rotation=rr.Quaternion(xyzw=qxyzw),
                from_parent=True,
                axis_length=0.1,
            ),
        )
        rr.log(f"{parent}/camera", rr.ViewCoordinates.RDF, static=True)

        camera_id: int = image.camera_id

        camera = cameras[camera_id]

        width = camera.width
        height = camera.height

        path = f"{parent}/camera/image"
        image_plane_distance = 0.1

        if camera.model == "SIMPLE_RADIAL":
            rr.log(
                path,
                rr.Pinhole(
                    resolution=[width, height],
                    focal_length=[camera.params[0], camera.params[0]],
                    principal_point=[camera.params[1], camera.params[2]],
                    image_plane_distance=image_plane_distance,
                ),
            )
        else:
            rr.log(
                f"{parent}/camera/image",
                rr.Pinhole(
                    resolution=[width, height],
                    focal_length=[width / 5, width / 5],
                    principal_point=[width / 2, height / 2],
                    image_plane_distance=0.1,
                ),
            )

        if len(image_dir) > 0:
            name = image.name
            fp = os.path.join(image_dir, name)
            bgr = cv2.imread(str(fp))
            bgr = cv2.resize(bgr, (width, height))
            rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)
            rr.log(
                f"{parent}/camera/image",
                rr.Image(rgb).compress(jpeg_quality=75),  # pyright: ignore
            )


def visualize(config: Config):
    cameras, images, points3D = read_model(  # pyright: ignore
        Path(config.sparse_path), ext=".bin" if config.input_type == "BIN" else ".txt"
    )

    assert type(cameras) is dict
    cameras = cast(CameraDict, cameras)
    assert type(images) is dict
    images = cast(ImageDict, images)
    assert type(points3D) is dict
    points3D = cast(Point3DDict, points3D)

    setup_coordinates()

    if config.draw_points:
        visualize_points3D(points3D)

    if config.all_frame:
        visualize_all_frame(images)

    visualize_images(cameras, images, config.image_dir)


def main() -> None:
    """
    parser = argparse.ArgumentParser(description="")
    parser.add_argument("--dataset", required=True, type=str)
    parser.add_argument("--image_dir", required=False, default="", type=str)
    parser.add_argument(
        "--input_type",
        type=str,
        default="BIN",
        choices=["BIN", "TXT"],
        help="input type",
    )
    parser.add_argument("--all_frames", action="store_true")
    rr.script_add_args(parser)
    args = parser.parse_args()

    blueprint = rrb.Vertical(
        rrb.Spatial3DView(name="3D", origin="/"),
        rrb.Spatial2DView(name="Camera", origin="/camera/image"),
        row_shares=[3, 2],
    )

    rr.script_setup(args, "rerun_vis_colmap_sparse", default_blueprint=blueprint)

    visualize(args)
    """

    config = tyro.cli(Config)

    blueprint = rrb.Vertical(
        rrb.Spatial3DView(name="3D", origin="/"),
        rrb.Spatial2DView(name="Camera", origin="/camera/image"),
        row_shares=[3, 2],
    )
    rr.init("rerun_vis_colmap_sparse", default_blueprint=blueprint, spawn=False)

    rr.connect(config.host)
    try:
        visualize(config)
    finally:
        rr.disconnect()


if __name__ == "__main__":
    main()
