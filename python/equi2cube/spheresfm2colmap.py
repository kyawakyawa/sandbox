import argparse
import math
import os
import os.path as pt
from pathlib import Path
from typing import cast

import cv2
import numpy as np
import torch
from tqdm import tqdm

from read_write_model import Image as CImage
from read_write_model import qvec2rotmat, read_model, rotmat2qvec

ImageDict = dict[int, CImage]

DEVICE = torch.device("cuda:0")
FTYPE = torch.float32
DIRECTION_NAMES = ["front", "right", "back", "left", "up", "down"]


def create_grid_old(coord2camera: torch.Tensor, tile_w: int, tile_h: int):
    coord_x, coord_y = torch.meshgrid(
        torch.arange(tile_w).to(FTYPE).to(DEVICE),
        torch.arange(tile_h).to(FTYPE).to(DEVICE),
        indexing="xy",
    )

    coord = torch.stack(
        [coord_x, coord_y, torch.ones_like(coord_x), torch.ones_like(coord_x)],
        dim=-1,
    )
    point = (coord2camera[None, None, :, :] @ coord[:, :, :, None]).squeeze(-1)
    point = point[..., :3] / point[..., 3:]
    point = point / torch.norm(point, p=2, dim=-1, keepdim=True)

    theta = torch.acos(point[..., 1])
    phi = torch.atan2(point[..., 2], point[..., 0])

    grid = torch.stack(
        [
            -(((phi + math.pi) / (2 * math.pi)) * 2 - 1),
            -((theta / math.pi) * 2 - 1),
        ],
        dim=-1,
    )

    return grid


def create_grid(coord2camera: torch.Tensor, tile_w: int, tile_h: int):
    coord_x, coord_y = torch.meshgrid(
        torch.arange(tile_w).to(FTYPE).to(DEVICE),
        torch.arange(tile_h).to(FTYPE).to(DEVICE),
        indexing="xy",
    )

    coord = torch.stack(
        [coord_x, coord_y, torch.ones_like(coord_x), torch.ones_like(coord_x)],
        dim=-1,
    )
    point = (coord2camera[None, None, :, :] @ coord[:, :, :, None]).squeeze(-1)
    point = point[..., :3] / point[..., 3:]
    point = point / torch.norm(point, p=2, dim=-1, keepdim=True)

    # thetaは[0, pi]
    theta = torch.acos(point[..., 1])
    phi = torch.atan2(point[..., 2], point[..., 0])

    # phiを [-pi,pi] から [-pi/2,3pi/2]に変換する
    phi = torch.where(phi < -torch.pi / 2, phi + 2 * torch.pi, phi)

    # grid_sampleでの座標を(x,y)とする。
    # phiが3*pi/2から-pi/2に動くときxは-1から1に線形に動く
    # thetaがpiから0に動くときyは-1から1に線形に動く

    x = 0.5 - phi / torch.pi
    y = 1 - 2 * theta / torch.pi

    x = torch.clamp(x, -1.0, 1.0)
    y = torch.clamp(y, -1.0, 1.0)

    grid = torch.stack([x, y], dim=-1)

    return grid

def to_cube(args: argparse.Namespace, image_path: str):
    tile_w = args.size
    tile_h = args.size

    img_np = cv2.imread(image_path)

    img = torch.from_numpy(img_np).to(DEVICE).permute(2, 0, 1)
    img = img.to(FTYPE) / 255

    fx = fy = tile_w / (2 * math.tan(math.pi * 0.5 / 2))
    cx = tile_w / 2
    cy = tile_h / 2

    intrinsic = torch.eye(4).to(FTYPE).to(DEVICE)
    intrinsic[0, 0] = fx
    intrinsic[1, 1] = fy
    intrinsic[0, 2] = cx
    intrinsic[1, 2] = cy

    grids = torch.zeros((6, tile_h, tile_w, 2), dtype=FTYPE, device=DEVICE)
    deg90 = math.pi / 2
    additional_extrinsics = (
        torch.eye(4).to(FTYPE).to(DEVICE)[None, :, :].repeat(6, 1, 1)
    )

    # front
    R = torch.eye(4).to(FTYPE).to(DEVICE)
    coord2camera = torch.linalg.inv(intrinsic @ R)
    grids[0, ...] = create_grid(coord2camera, tile_w, tile_h)
    additional_extrinsics[0, ...] = R

    # right
    R = torch.eye(4).to(FTYPE).to(DEVICE)
    R[0, 0] = math.cos(deg90)
    R[0, 2] = -math.sin(deg90)
    R[2, 0] = math.sin(deg90)
    R[2, 2] = math.cos(deg90)
    coord2camera = torch.linalg.inv(intrinsic @ R)
    grids[1, ...] = create_grid(coord2camera, tile_w, tile_h)
    additional_extrinsics[1, ...] = R

    # back
    R = torch.eye(4).to(FTYPE).to(DEVICE)
    R[0, 0] = math.cos(deg90 * 2)
    R[0, 2] = -math.sin(deg90 * 2)
    R[2, 0] = math.sin(deg90 * 2)
    R[2, 2] = math.cos(deg90 * 2)
    coord2camera = torch.linalg.inv(intrinsic @ R)
    grids[2, ...] = create_grid(coord2camera, tile_w, tile_h)
    additional_extrinsics[2, ...] = R

    # left
    R = torch.eye(4).to(FTYPE).to(DEVICE)
    R[0, 0] = math.cos(deg90 * 3)
    R[0, 2] = -math.sin(deg90 * 3)
    R[2, 0] = math.sin(deg90 * 3)
    R[2, 2] = math.cos(deg90 * 3)
    coord2camera = torch.linalg.inv(intrinsic @ R)
    grids[3, ...] = create_grid(coord2camera, tile_w, tile_h)
    additional_extrinsics[3, ...] = R

    # up
    R = torch.eye(4).to(FTYPE).to(DEVICE)
    R[2, 2] = math.cos(deg90)
    R[2, 1] = -math.sin(deg90)
    R[1, 2] = math.sin(deg90)
    R[1, 1] = math.cos(deg90)
    coord2camera = torch.linalg.inv(intrinsic @ R)
    grids[4, ...] = create_grid(coord2camera, tile_w, tile_h)
    additional_extrinsics[4, ...] = R

    # down
    R = torch.eye(4).to(FTYPE).to(DEVICE)
    R[2, 2] = math.cos(deg90)
    R[2, 1] = math.sin(deg90)
    R[1, 2] = -math.sin(deg90)
    R[1, 1] = math.cos(deg90)
    coord2camera = torch.linalg.inv(intrinsic @ R)
    grids[5, ...] = create_grid(coord2camera, tile_w, tile_h)
    additional_extrinsics[5, ...] = R

    imgs = img[None]
    imgs = imgs.expand(6, -1, -1, -1)

    sampled = torch.nn.functional.grid_sample(imgs, grids, align_corners=True)

    sampled_np = (sampled.permute(0, 2, 3, 1) * 255).to(torch.uint8).cpu().numpy()
    additional_extrinsics_np = additional_extrinsics.cpu().numpy()

    intrinsics_np = (
        torch.tensor([fx, fy, cx, cy])[None, :].to(FTYPE).expand(6, -1).cpu().numpy()
    )

    return sampled_np, additional_extrinsics_np, intrinsics_np


def colmap_pose_to_44(txyz: np.ndarray, qwxyz: np.ndarray):
    ret = np.eye(4).astype(txyz.dtype)
    ret[:3, 3] = txyz

    ret[:3, :3] = qvec2rotmat(qwxyz)
    return ret


def to_colmap_pose(t44: np.ndarray):
    R = t44[:3, :3]
    t = t44[:3, 3]

    qvec = rotmat2qvec(R)

    return qvec, t


def write_colmap_sparse_txt(
    dir: str,
    names: list[str],
    txyzs: np.ndarray,
    qwxyzs: np.ndarray,
    intrinsics: np.ndarray,
    single_camera: bool,
    size: int,
):
    # points3D.txtは空ファイル
    with open(pt.join(dir, "points3D.txt"), "w") as f:
        pass

    # ### cameras.txt ###
    camera_header = (
        "/home/okawa/data/realsense_test/colmap_data_t1/sparse_prior/model/cameras.txt"
    )
    _intrinsics = intrinsics
    if single_camera:
        _intrinsics = intrinsics[:1]

    camera_header = (
        "# Camera list with one line of data per camera:\n"
        "#   CAMERA_ID, MODEL, WIDTH, HEIGHT, PARAMS[]\n"
    )
    camera_header += f"# Number of cameras: {len(_intrinsics)}\n"

    with open(pt.join(dir, "cameras.txt"), "w") as f:
        f.write(camera_header)

        for camera_id, intrinsic in enumerate(_intrinsics):
            fx, fy, cx, cy = intrinsic
            assert (fx - fy) < 1e-6
            f.write(f"{camera_id + 1} ")
            f.write("SIMPLE_RADIAL ")
            f.write(f"{size} {size} ")
            f.write(f"{fx} {cx} {cy} {0.0}\n")
    # ###################

    # ### images.txt ###

    image_header = (
        "# Image list with two lines of data per image:\n"
        "#   IMAGE_ID, QW, QX, QY, QZ, TX, TY, TZ, CAMERA_ID, NAME\n"
        "#   POINTS2D[] as (X, Y, POINT3D_ID)\n"
    )
    image_header += f"# Number of images: {len(names)}\n"

    with open(pt.join(dir, "images.txt"), "w") as f:
        f.write(image_header)

        for image_id, (name, txyz, qwxyz) in enumerate(zip(names, txyzs, qwxyzs)):
            camera_id = image_id + 1

            f.write(f"{image_id + 1} ")
            f.write(" ".join(map(str, qwxyz)) + " ")
            f.write(" ".join(map(str, txyz)) + " ")
            f.write(f"{camera_id} {name}\n\n")

    # ##################


# Image list with two lines of data per image:
#   IMAGE_ID, QW, QX, QY, QZ, TX, TY, TZ, CAMERA_ID, NAME
#   POINTS2D[] as (X, Y, POINT3D_ID)
# Number of images: 127, mean observations per image: 710.67716535433067


def convert_all_images(args: argparse.Namespace, images: ImageDict):
    n = len(images)
    num_direction = 6
    txyzs = np.zeros((n * num_direction, 3))
    qwxyzs = np.zeros((n * num_direction, 4))
    intrinsics = np.zeros((n * num_direction, 4))
    names = []

    for img_cnt, image in tqdm(enumerate(images.values()), total=n):
        image_path = pt.join(args.image_dir, image.name)
        cube_images, additional_extrinsics, _intrinsics = to_cube(args, image_path)

        assert len(cube_images) == num_direction

        for i, cube_image in enumerate(cube_images):
            se = os.path.splitext(image.name)
            new_image_name = se[0] + f"-{DIRECTION_NAMES[i]}" + se[1]

            output_path = pt.join(args.images_output_dir, new_image_name)
            tqdm.write("Write image: " + output_path)
            cv2.imwrite(output_path, cube_image)

            # Colmap用の情報を記録
            _qvec, _t = to_colmap_pose(
                additional_extrinsics[i] @ colmap_pose_to_44(image.tvec, image.qvec)
            )
            txyzs[img_cnt * num_direction + i] = _t
            qwxyzs[img_cnt * num_direction + i] = _qvec
            intrinsics[img_cnt * num_direction + i] = _intrinsics[i]
            names.append(new_image_name)

    return names, txyzs, qwxyzs, intrinsics


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-i", "--input_dir", type=str, required=True, help="sphare sfm sparse"
    )
    parser.add_argument("-I", "--image_dir", type=str, required=True, help="image dir")
    parser.add_argument(
        "--input_type",
        type=str,
        default="BIN",
        choices=["BIN", "TXT"],
        help="input type",
    )
    parser.add_argument(
        "-o", "--output_dir", type=str, required=True, help="output colmap sparse"
    )
    parser.add_argument(
        "-O",
        "--images_output_dir",
        type=str,
        required=True,
        help="output colmap images",
    )
    parser.add_argument("-s", "--size", type=int, default=1600, help="tile size")
    parser.add_argument(
        "--single_camera", action="store_true", help="use single camera model"
    )
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)
    os.makedirs(args.images_output_dir, exist_ok=True)

    input_dir: str = args.input_dir
    output_dir: str = args.output_dir

    _, images, _ = read_model(  # pyright: ignore
        Path(input_dir), ext=".bin" if args.input_type == "BIN" else ".txt"
    )

    images = cast(ImageDict, images)

    names, txyzs, qwxyzs, intrinsics = convert_all_images(args, images)

    write_colmap_sparse_txt(
        output_dir, names, txyzs, qwxyzs, intrinsics, args.single_camera, args.size
    )


if __name__ == "__main__":
    main()
