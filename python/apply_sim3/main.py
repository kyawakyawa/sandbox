import argparse

import lib.align
import lib.create_case
import numpy as np
import pypose
import rerun as rr  # pip install rerun-sdk
import rerun.blueprint as rrb
import torch
from pypose import mat2SO3


def tomat44(scale: float, rot: np.ndarray, trans: np.ndarray):
    """
    rot: 3x3
    trans: 3
    """
    ret = np.eye(4, dtype=rot.dtype)

    ret[:, :3, :3] = s * rot
    ret[:, :3, 3] = trans

    return ret


def draw(cam_rot: np.ndarray, cam_cen_world: np.ndarray, points: np.ndarray):
    """
    cam_rot: n x 3 x 3
    cam_cen_world: n x 3
    """

    n = cam_rot.shape[0]
    # rotation of world to local
    rot_l2w = cam_rot.transpose(0, 2, 1)
    # translation of world to local
    trans_l2w = (-rot_l2w @ cam_cen_world[:, :, None]).squeeze(axis=-1)

    for i in range(n):
        txyz = trans_l2w[i]
        rot_l2w_tensor = torch.from_numpy(rot_l2w[i])

        qxyzw = mat2SO3(rot_l2w_tensor).data.cpu().numpy()

        bp_path = f"/camera{i:04d}"

        rr.log(
            bp_path,
            rr.Transform3D(
                translation=txyz,
                rotation=rr.Quaternion(xyzw=qxyzw),
                from_parent=True,
            ),
        )
        rr.log(
            bp_path, rr.ViewCoordinates.RDF, static=True
        )  # X=Right, Y=Down, Z=Forward

        rr.log(
            bp_path + "/image",
            rr.Pinhole(
                resolution=[960, 512],
                focal_length=[960 / 6, 960 / 6],
                principal_point=[480, 256],
                image_plane_distance=0.1,
            ),
        )
    point_colors = np.full((points.shape[0], 3), 255, dtype=np.uint8)
    rr.log(
        "points",
        rr.Points3D(points, colors=point_colors),
    )


def main():
    parser = argparse.ArgumentParser(description="Appling Sim3 test")
    rr.script_add_args(parser)
    args = parser.parse_args()

    blueprint = rrb.Vertical(
        rrb.Spatial3DView(name="3D", origin="/"),
    )

    rr.script_setup(args, "appling_sim3_test", default_blueprint=blueprint)

    rr.log("/", rr.ViewCoordinates.RIGHT_HAND_Y_DOWN, static=True)

    cam_a_rot, cam_a_cen_world, cam_b_rot, cam_b_cen_world = (
        lib.create_case.create_simple_case()
    )

    rot, trans, trans_error, scale = lib.align.align(
        cam_a_cen_world.transpose(1, 0), cam_b_cen_world.transpose(1, 0)
    )
    print("scale: ", scale)

    points_a, points_b = lib.create_case.create_simple_points(100, scale, rot, trans)

    rr.set_time_seconds("stable_time", 1)
    draw(cam_a_rot, cam_a_cen_world, points_a)

    rr.set_time_seconds("stable_time", 2)
    draw(cam_b_rot, cam_b_cen_world, points_b)

    rr.script_teardown(args)


if __name__ == "__main__":
    main()
