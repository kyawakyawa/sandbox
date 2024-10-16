import argparse
import os
import os.path
import shutil
import sqlite3
from pathlib import Path
from typing import cast

from read_write_model import Image as CImage
from read_write_model import read_model, write_images_binary, write_images_text

ImageDict = dict[int, CImage]

MAX_NUM_IMAGES = 2147483647


def collect_all_image_ids(conn: sqlite3.Connection):
    cur = conn.cursor()
    ret = {}
    try:
        cur.execute("SELECT image_id, name FROM images")

        ret = {k: v for k, v in cur.fetchall()}

        # for row in cur.fetchall():
        #     ret[row[0]] = row[1]
        # ret = set([row[0] for row in cur.fetchall()])

    finally:
        cur.close()
    return ret


def pair_id2image_ids(pair_id: int):
    image_id2 = pair_id % MAX_NUM_IMAGES
    image_id1 = (pair_id - image_id2) // MAX_NUM_IMAGES

    return image_id1, image_id2


def collect_has_connection_image_ids(conn: sqlite3.Connection):
    cur = conn.cursor()
    ret = set()
    try:
        cur.execute("SELECT pair_id FROM two_view_geometries WHERE rows > 0")

        pairs = [pair_id2image_ids(row[0]) for row in cur.fetchall()]
        print("num two view geometries: ", len(pairs))
        # for id1, id2 in pairs:
        #     print(id1, " ", id2)

        ret = set([v[0] for v in pairs] + [v[1] for v in pairs])

    finally:
        cur.close()
    return ret


def main():

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-d",
        "--database_path",
        type=str,
        required=True,
        help="colmap database path to be corrected",
    )
    parser.add_argument(
        "-i",
        "--image_dir",
        type=str,
        required=False,
        default="",
        help="image dir path. If you want to move non connected images to other directory, please specify this param.",
    )
    parser.add_argument(
        "-o",
        "--out_image_dir",
        type=str,
        required=False,
        default="",
        help="Image's destination directory. If you want to move non connected images to other directory, please specify this param.",
    )
    parser.add_argument(
        "-s",
        "--sparse",
        type=str,
        required=False,
        default="",
        help="Colmap sparse model dir. If you want to edit sparse model, please specify this param.",
    )
    parser.add_argument(
        "--model_input_type",
        type=str,
        default="BIN",
        choices=["BIN", "TXT"],
        help="input type",
    )

    args = parser.parse_args()

    conn = sqlite3.connect(args.database_path)

    try:
        image_ids = collect_all_image_ids(conn)
        has_connection_image_ids = collect_has_connection_image_ids(conn)

        for hcii in has_connection_image_ids:
            del image_ids[hcii]

        for non_connected_image_id, name in image_ids.items():
            print("image_id: ", non_connected_image_id, ", name: ", name)
        print(
            f"{len(image_ids)} image{ "s are" if len(image_ids) > 1 else " is"} non connected to any image"
        )

        if len(args.image_dir) > 0 and len(args.out_image_dir) > 0:
            for name in image_ids.values():
                if os.path.exists(os.path.join(args.image_dir, name)):
                    os.makedirs(
                        os.path.join(args.out_image_dir, os.path.dirname(name)),
                        exist_ok=True,
                    )
                    shutil.move(
                        os.path.join(args.image_dir, name),
                        os.path.join(args.out_image_dir, name),
                    )

        if len(args.sparse) > 0:
            _, colmap_images, _ = read_model(  # pyright: ignore
                Path(args.sparse),
                ext=".bin" if args.model_input_type == "BIN" else ".txt",
            )
            colmap_images = cast(ImageDict, colmap_images)

            names_delete = set([name for name in image_ids.values()])

            out_images: ImageDict = {}
            out_images = {
                image_id: image
                for image_id, image in colmap_images.items()
                if image.name not in names_delete
            }

            if args.model_input_type == "BIN":
                write_images_binary(out_images, Path(args.sparse) / "images.bin")
            else:
                assert args.model_input_type == "TXT"
                write_images_text(out_images, Path(args.sparse) / "images.txt")

    finally:
        conn.close()


if __name__ == "__main__":
    main()
