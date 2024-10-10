import argparse
import glob
import math
import os.path as pt

import cv2
import torch

DEVICE = torch.device("cuda:0")
FTYPE = torch.float32


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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input_dir", type=str, required=True)
    parser.add_argument("-o", "--output_dir", type=str, required=True)
    parser.add_argument("-s", "--size", type=int, default=1600, help="tile size")
    args = parser.parse_args()

    input_dir: str = args.input_dir
    output_dir: str = args.output_dir

    img_pathes = glob.glob(pt.join(input_dir, "*"))
    img_pathes.sort()

    for img_path in img_pathes:
        img_np = cv2.imread(img_path)

        img = torch.from_numpy(img_np).to(DEVICE).permute(2, 0, 1)
        img = img.to(FTYPE) / 255

        tile_w = args.size
        tile_h = args.size

        fx = fy = tile_w / (2 * math.tan(math.pi * 0.5 / 2))
        cx = tile_w / 2
        cy = tile_h / 2

        intrinsic = torch.eye(4).to(FTYPE).to(DEVICE)
        intrinsic[0, 0] = fx
        intrinsic[1, 1] = fy
        intrinsic[0, 2] = cx
        intrinsic[1, 2] = cy

        grids = torch.zeros((5, tile_h, tile_w, 2), dtype=FTYPE, device=DEVICE)
        deg90 = math.pi / 2

        # front
        R = torch.eye(4).to(FTYPE).to(DEVICE)
        coord2camera = torch.linalg.inv(intrinsic @ R)
        grids[0, ...] = create_grid(coord2camera, tile_w, tile_h)

        # right
        R = torch.eye(4).to(FTYPE).to(DEVICE)
        R[0, 0] = math.cos(deg90)
        R[0, 2] = -math.sin(deg90)
        R[2, 0] = math.sin(deg90)
        R[2, 2] = math.cos(deg90)
        coord2camera = torch.linalg.inv(intrinsic @ R)
        grids[1, ...] = create_grid(coord2camera, tile_w, tile_h)

        # back
        R = torch.eye(4).to(FTYPE).to(DEVICE)
        R[0, 0] = math.cos(deg90 * 2)
        R[0, 2] = -math.sin(deg90 * 2)
        R[2, 0] = math.sin(deg90 * 2)
        R[2, 2] = math.cos(deg90 * 2)
        coord2camera = torch.linalg.inv(intrinsic @ R)
        grids[2, ...] = create_grid(coord2camera, tile_w, tile_h)

        # left
        R = torch.eye(4).to(FTYPE).to(DEVICE)
        R[0, 0] = math.cos(deg90 * 3)
        R[0, 2] = -math.sin(deg90 * 3)
        R[2, 0] = math.sin(deg90 * 3)
        R[2, 2] = math.cos(deg90 * 3)
        coord2camera = torch.linalg.inv(intrinsic @ R)
        grids[3, ...] = create_grid(coord2camera, tile_w, tile_h)

        # up
        R = torch.eye(4).to(FTYPE).to(DEVICE)
        R[2, 2] = math.cos(deg90)
        R[2, 1] = -math.sin(deg90)
        R[1, 2] = math.sin(deg90)
        R[1, 1] = math.cos(deg90)
        coord2camera = torch.linalg.inv(intrinsic @ R)
        grids[4, ...] = create_grid(coord2camera, tile_w, tile_h)

        imgs = img[None]
        imgs = imgs.expand(5, -1, -1, -1)

        sampled = torch.nn.functional.grid_sample(imgs, grids, align_corners=True)

        common, _ = pt.splitext(pt.basename(img_path))

        common = pt.join(output_dir, common)

        cv2.imwrite(
            common + "-front.png",
            (sampled[0] * 255).to(torch.uint8).permute(1, 2, 0).cpu().numpy(),
        )

        cv2.imwrite(
            common + "-right.png",
            (sampled[1] * 255).to(torch.uint8).permute(1, 2, 0).cpu().numpy(),
        )

        cv2.imwrite(
            common + "-back.png",
            (sampled[2] * 255).to(torch.uint8).permute(1, 2, 0).cpu().numpy(),
        )

        cv2.imwrite(
            common + "-left.png",
            (sampled[3] * 255).to(torch.uint8).permute(1, 2, 0).cpu().numpy(),
        )

        cv2.imwrite(
            common + "-up.png",
            (sampled[4] * 255).to(torch.uint8).permute(1, 2, 0).cpu().numpy(),
        )

        print("intrinsic: ", intrinsic)


if __name__ == "__main__":
    main()
