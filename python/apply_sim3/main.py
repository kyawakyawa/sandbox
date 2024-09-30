import argparse

import lib.align
import lib.create_case
import numpy as np
import rerun as rr  # pip install rerun-sdk
import rerun.blueprint as rrb


def tomat44(scale: float, rot: np.ndarray, trans: np.ndarray):
    """
    rot: 3x3
    trans: 3
    """
    ret = np.eye(4, dtype=rot.dtype)

    ret[:, :3, :3] = s * rot
    ret[:, :3, 3] = trans

    return ret


def draw(cam_rot: np.ndarray, cam_cen_world: np.ndarray):
    """
    cam_rot: n x 3 x 3
    cam_cen_world: n x 3
    """

    # rotation of world to local
    rot_l2w = cam_rot.transpose(0, 2, 1)

    # translation of world to local
    trans_l2w = (-rot_l2w @ cam_cen_world[:, :, None]).squeeze(axis=-1)


def main():
    parser = argparse.ArgumentParser(description="Appling Sim3 test")
    rr.script_add_args(parser)
    args = parser.parse_args()

    blueprint = rrb.Vertical(
        rrb.Spatial3DView(name="3D", origin="/"),
    )

    rr.script_setup(args, "appling_sim3_test", default_blueprint=blueprint)

    cam_a_rot, cam_a_cen_world, cam_b_rot, cam_b_cen_world = (
        lib.create_case.create_simple_case()
    )

    rot, trans, trans_error, scale = lib.align.align(
        cam_a_cen_world.transpose(1, 0), cam_b_cen_world.transpose(1, 0)
    )
    print("scale: ", scale)

    points_a, points_b = lib.create_case.create_simple_points(100, scale, rot, trans)

    rr.script_teardown(args)


if __name__ == "__main__":
    main()
