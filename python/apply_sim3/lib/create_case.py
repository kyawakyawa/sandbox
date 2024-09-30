import numpy as np


def create_simple_case():
    cam_a_cen_world = np.array(
        [[0, 0, 0], [1, 0, 0], [1, -1, 0], [0, -1, 0]], dtype=np.float64
    )
    cam_b_cen_world = np.array(
        [[0, -1, 1], [0, -1, 3], [0, -3, 3], [0, -3, 1]], dtype=np.float64
    )

    # rotations are common for all cameras
    cam_a_rot = np.array([[-1, 0, 0], [0, -1, 0], [0, 0, 1]], dtype=np.float64)[
        None, :, :
    ].repeat(4, axis=0)
    cam_b_rot = np.array([[0, 0, -1], [0, -1, 0], [-1, 0, 0]], dtype=np.float64)[
        None, :, :
    ].repeat(4, axis=0)

    return cam_a_rot, cam_a_cen_world, cam_b_rot, cam_b_cen_world


def create_simple_points(n: int, scale: float, rot: np.ndarray, trans: np.ndarray):

    # 0 < x < 1
    # -1 < y < 0
    # 0.5 < z < 1.5
    # で点をn個作成
    points = np.random.rand(n, 3)
    points[:, 0] = points[:, 0]
    points[:, 1] = -points[:, 1]
    points[:, 2] = points[:, 2] + 0.5

    points_applied_sim3 = (scale * rot[None] @ points[:, :, None]).squeeze(
        axis=-1
    ) + trans[None]

    return points, points_applied_sim3
