import argparse
import os
from pathlib import Path

import cv2
import rerun as rr  # pip install rerun-sdk
import rerun.blueprint as rrb

from read_write_model import Image as CImage
from read_write_model import read_model

ImageDict = dict[int, CImage]


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
                axis_length=0.1,
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
                image_plane_distance=0.1,
            ),
        )


def visualize_each_frame(images: ImageDict, image_dir: str):
    for image in images.values():
        name = image.name

        txyz = image.tvec
        qxyzw = image.qvec[[1, 2, 3, 0]]

        # COLMAP's camera transform is "camera from world"
        rr.log(
            "camera",
            rr.Transform3D(
                translation=txyz,
                rotation=rr.Quaternion(xyzw=qxyzw),
                from_parent=True,
                axis_length=0.1,
            ),
        )
        rr.log("camera", rr.ViewCoordinates.RDF, static=True)

        width = 960
        height = 540
        rr.log(
            "camera/image",
            rr.Pinhole(
                resolution=[width, height],
                focal_length=[width / 5, width / 5],
                principal_point=[width / 2, height / 2],
                image_plane_distance=0.1,
            ),
        )

        if len(image_dir) > 0:
            fp = os.path.join(image_dir, name)
            bgr = cv2.imread(str(fp))
            bgr = cv2.resize(bgr, (width, height))
            rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)
            rr.log(
                "camera/image",
                rr.Image(rgb).compress(jpeg_quality=75),  # pyright: ignore
            )


def visualize(args: argparse.Namespace) -> None:
    cameras, images, points3D = read_model(  # pyright: ignore
        Path(args.dataset), ext=".bin" if args.input_type == "BIN" else ".txt"
    )

    rr.log("/", rr.ViewCoordinates.RIGHT_HAND_Y_DOWN, static=True)

    visualize_all_frame(images)  # pyright: ignore

    visualize_each_frame(images, args.image_dir)  # pyright: ignore


def main() -> None:
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
    rr.script_add_args(parser)
    args = parser.parse_args()

    blueprint = rrb.Vertical(
        rrb.Spatial3DView(name="3D", origin="/"),
        rrb.Spatial2DView(name="Camera", origin="/camera/image"),
        row_shares=[3, 2],
    )

    rr.script_setup(args, "rerun_vis_colmap_sparse", default_blueprint=blueprint)

    visualize(args)


if __name__ == "__main__":
    main()
