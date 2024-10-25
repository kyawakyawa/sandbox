import argparse
import os
import sqlite3
from pathlib import Path
from typing import cast

import cv2
import numpy as np
import rerun as rr
import rerun.blueprint as rrb

from read_write_model import Image, Point3D, read_model

ImageDict = dict[int, Image]
Point3DDict = dict[int, Point3D]
MAX_NUM_IMAGES = 2147483647


def log_all_points(points3D: Point3DDict, parent: str = ""):
    xyzs = [point3D.xyz for point3D in points3D.values()]
    rgbs = [point3D.rgb for point3D in points3D.values()]
    radii = np.ones(len(xyzs)) * 0.01

    rr.log(f"{parent}/all_points", rr.Points3D(xyzs, colors=rgbs, radii=radii))


def draw_images(image: Image, parent: str = "", image_dir: str = ""):
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

    width = 960
    height = 540
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


def get_max_point_id(points3D: Point3DDict):
    ret: int = -1
    for point3D in points3D.values():
        id = point3D.id
        id = cast(int, id)
        ret = id if id > ret else ret
    return ret


def log_point_triangulator(
    images: ImageDict,
    points3D: Point3DDict,
    image_pairs: list[tuple[int, int]],
    parent="",
    image_dir="",
):
    for image_id1, image_id2 in image_pairs:
        image1 = images[image_id1]
        image2 = images[image_id2]

        visible1 = np.unique(np.array([image1.point3D_ids]))
        visible2 = np.unique(np.array([image2.point3D_ids]))
        # ↑-1 が入っているかも
        visible = np.intersect1d(visible1, visible2)
        visible = np.sort(visible)
        assert len(visible) > 0

        if visible[0] == -1:
            visible = visible[1:]

        if len(visible) <= 100:
            continue

        draw_images(image1, parent="/1", image_dir=image_dir)
        draw_images(image2, parent="/2", image_dir=image_dir)

        xyzs = [points3D[id].xyz for id in visible]
        # rgbs = [points3D[id].rgb for id in visible]
        rgbs = np.full(
            (len(xyzs), 3),
            255,
            dtype=np.uint8,
        )
        rgbs[:, 1] = 0
        rgbs[:, 2] = 0
        radii = np.ones(len(xyzs)) * 0.03
        rr.log(f"{parent}/point", rr.Points3D(xyzs, colors=rgbs, radii=radii))


def setup_coordinates():
    rr.log("/", rr.ViewCoordinates.RIGHT_HAND_Y_DOWN, static=True)


def pair_id2image_ids(pair_id: int):
    image_id2 = pair_id % MAX_NUM_IMAGES
    image_id1 = (pair_id - image_id2) // MAX_NUM_IMAGES

    return image_id1, image_id2


def load_two_view_geometory_from_db(conn: sqlite3.Connection):
    cur = conn.cursor()
    pairs: list[tuple[int, int]] = []
    try:
        cur.execute("SELECT pair_id FROM two_view_geometries WHERE rows > 0")

        pairs = [pair_id2image_ids(row[0]) for row in cur.fetchall()]

    finally:
        cur.close()
    return pairs


def load_two_view_geometory(database_path: str):
    conn = sqlite3.connect(database_path)
    pairs: list[tuple[int, int]] = []
    try:
        pairs = load_two_view_geometory_from_db(conn)
    finally:
        conn.close()
    return pairs


def main():
    parser = argparse.ArgumentParser(
        description="Visualize the output of COLMAP's sparse reconstruction on a video."
    )
    parser.add_argument(
        "-d", "--database", type=str, help="Path to the COLMAP database path"
    )
    parser.add_argument(
        "-i", "--input_path", type=str, help="Path to the COLMAP sparse mode directory"
    )
    parser.add_argument(
        "-I",
        "--image_dir",
        type=str,
        required=False,
        default="",
        help="Path to the image dir",
    )
    parser.add_argument(
        "--input_type",
        type=str,
        default="BIN",
        choices=["BIN", "TXT"],
        help="input type",
    )
    parser.add_argument("--all_points", action="store_true", help="log all points")

    rr.script_add_args(parser)
    args = parser.parse_args()

    blueprint = rrb.Vertical(
        rrb.Spatial3DView(name="3D", origin="/"),
        rrb.Horizontal(
            rrb.Spatial2DView(name="Camera", origin="/1/camera/image"),
            rrb.Spatial2DView(name="Camera", origin="/2/camera/image"),
            rrb.TimeSeriesView(origin="/plot"),
        ),
        row_shares=[3, 2],
    )
    rr.script_setup(args, "vis_point_triangulator", default_blueprint=blueprint)

    setup_coordinates()

    _, images, points3D = read_model(
        Path(args.input_path), ext=".bin" if args.input_type == "BIN" else ".txt"
    )  # pyright: ignore

    assert type(images) is dict
    assert type(points3D) is dict

    if args.all_points:
        log_all_points(points3D)

    image_pairs = load_two_view_geometory(args.database)

    log_point_triangulator(images, points3D, image_pairs, image_dir=args.image_dir)


if __name__ == "__main__":
    main()
