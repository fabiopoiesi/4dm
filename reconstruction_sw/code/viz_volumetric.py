import open3d as o3d
import numpy as np
import os
import json
import quaternion as quat

###################################################
############### MAIN
###################################################

do_white_background = False
do_save = False

dataset_name = 'easy'
mesh_folder = '../results/volumetric_meshes'
dataset_root_folder = '/home/poiesi/data/datasets/4dm/'

if do_white_background:
    dest_folder = os.path.join('../results/volumetric_viz', dataset_name, 'white')
else:
    dest_folder = os.path.join('../results/volumetric_viz', dataset_name, 'black')
if not os.path.isdir(dest_folder):
    os.makedirs(dest_folder)

mobiles = {1: '4846a8bc1c6f287a725111d3f60f2385',
           2: 'cc74017ee8081c07917f03332f92ede6',
           3: 'd7ec5aa63b32a4878ca037e2a7fec252',
           4: '172673834746e6d591b4b00599c98ffa',
           5: '378d43cea3872b0551900a15c92e0ec4',
           6: '642a3152b339263852fffe527a9de347'}

# corrections computed with https://github.com/fabiopoiesi/dip
pose_corrections = [[[ 1., 0., 0., 0.],
                     [ 0., 1., 0., 0.],
                     [ 0., 0., 1., 0.],
                     [ 0., 0., 0., 1.]],
                    [[ 0.99905457, -0.02676729, -0.0342561,   0.15728716],
                     [ 0.02700194,  0.99961486,  0.00640553, -0.01462191],
                     [ 0.03407145, -0.00732446,  0.99939256,  0.00472023],
                     [ 0.,          0.,          0.,          1.,        ]],
                    [[ 0.99960058, -0.00509856,  0.02779724,  0.14406139],
                     [ 0.00464171,  0.9998535,   0.01647517, -0.00341851],
                     [-0.02787717, -0.01633956,  0.9994778,  -0.03475881],
                     [ 0.,          0.,         0.,          1.,        ]],
                    [[ 9.98765432e-01, -4.96677155e-02, -8.54867548e-04,  1.72896007e-01],
                     [ 4.96718223e-02,  9.98749038e-01,  5.75062066e-03, -6.17926371e-02],
                     [ 5.68177950e-04, -5.78598396e-03,  9.99983100e-01,  1.00602630e-01],
                     [ 0.00000000e+00,  0.00000000e+00,  0.00000000e+00,  1.00000000e+00]],
                    [[ 0.99998321, -0.00385142, -0.00432897,  0.19584433],
                     [ 0.00391902,  0.99986879,  0.01571766, -0.0033018 ],
                     [ 0.00426787, -0.01573436,  0.9998671,   0.0189917 ],
                     [ 0.,          0.,          0.,          1.        ]],
                    [[0.99847665, - 0.05491205,  0.00538861,  0.218133],
                     [0.05486804,  0.99846156,  0.00800104,  0.00688609],
                     [-0.00581968, - 0.00769319,  0.99995347, - 0.02124214],
                     [0., 0., 0., 1.]]
                    ]

if not os.path.isdir(dest_folder):
    os.mkdir(dest_folder)

mobile_viz_points = 0.4 * np.array([[-0.5, -0.5, 1], [0.5, -0.5, 1], [-0.5, 0.5, 1], [0.5, 0.5, 1], [0, 0, 0]])
mobile_viz_lines = [[0, 1], [2, 3], [2, 0], [1, 3], [4, 0], [4, 1], [4, 2], [4, 3]]
mobile_viz_colors = [[1, 0, 0], [1, 0, 0], [1, 0, 0], [1, 0, 0], [1, 1, 0], [1, 1, 0], [1, 1, 0], [1, 1, 0]]
if do_white_background:
    mobile_viz_colors = [[0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0], [1, 0, 0], [1, 0, 0], [1, 0, 0], [1, 0, 0]]

vis = o3d.visualization.Visualizer()
vis.create_window(visible=True, width=1920, height=1080-240)
vis.get_render_option().background_color = [0, 0, 0]
if do_white_background:
    vis.get_render_option().background_color = [1, 1, 1]
flag_viz_camera_rotation = True

vis.update_renderer()

# init mobile captures
mobile_captures = {}
for mobile_number, mobile in mobiles.items():
    mobile_captures[mobile_number] = {'im': [],
                                      'position': [],
                                      'rotation': [],
                                      'principal_point': [],
                                      'focal_length': [],
                                      'K': [],
                                      'frustum': [],
                                      'person_pose': []}

files = os.listdir(os.path.join(mesh_folder, dataset_name))
files.sort()

local_frame = o3d.geometry.TriangleMesh.create_coordinate_frame(size=.2)
vis.add_geometry(local_frame)

mesh = o3d.geometry.TriangleMesh()

c = np.asarray([1.00563633, 0, -1.10522775])
b = .9
h = 1.9
voxel_res = 220
X, Y, Z = np.meshgrid(np.linspace(c[0] - b, c[0] + b, voxel_res),
                   np.linspace(0, -h, voxel_res),
                   np.linspace(c[2] - b, c[2] + b, voxel_res))

c = 0
for mf in files:

    fr = int(mf.split('.')[0])

    # CAMERAS
    for mobile_number, mobile in mobiles.items():

        json_file = os.path.join(dataset_root_folder, dataset_name, mobile, '%04d.bin' % fr)

        # mobile pose
        with open(json_file, 'r') as jf:
            json_text = jf.read()
            meta = json.loads(json_text)

        pp = np.asarray([meta['principalPoint']['x'], meta['principalPoint']['y']])
        fl = np.asarray([meta['focalLength']['x'], meta['focalLength']['y']])

        rotation = quat.quaternion(meta['rotation']['w'],
                                   -meta['rotation']['x'],
                                   meta['rotation']['y'],
                                   -meta['rotation']['z'])

        C = np.asarray([meta['position']['x'],
                             -meta['position']['y'],
                             meta['position']['z']])

        # http://ksimek.github.io/2012/08/22/extrinsic/
        K = np.asarray([[fl[0], 0, pp[0]],
                        [0, fl[1], pp[1]],
                        [0, 0, 1]])

        R = quat.as_rotation_matrix(rotation)

        T = np.zeros((4, 4))
        T[:3,:3] = R
        T[:3, 3] = C
        T[3, 3] = 1

        mpts = mobile_viz_points.T
        Tnew = pose_corrections[mobile_number-1] @ T
        mobile_pose_world = (Tnew @ np.r_[mpts, np.ones((1, mpts.shape[1]))])[:3].T

        if c > 0:
            [vis.remove_geometry(l) for l in mobile_captures[mobile_number]['frustum']]
        frustum_to_viz = []
        line_mesh = LineMesh(mobile_pose_world, mobile_viz_lines, mobile_viz_colors, radius=0.02)
        line_mesh_geoms = line_mesh.cylinder_segments
        mobile_captures[mobile_number]['frustum'] = line_mesh_geoms
        [vis.add_geometry(l) for l in mobile_captures[mobile_number]['frustum']]

    # MESH
    vis.remove_geometry(mesh)

    mesh = o3d.io.read_triangle_mesh(os.path.join(mesh_folder, dataset_name, mf))

    triangles = np.asarray(mesh.vertices)

    mesh.compute_vertex_normals()

    vis.add_geometry(mesh)

    if flag_viz_camera_rotation:
        vis.run()
        camera_parameter = vis.get_view_control().convert_to_pinhole_camera_parameters()
        flag_viz_camera_rotation = 0

    vis.get_view_control().convert_from_pinhole_camera_parameters(camera_parameter)
    vis.poll_events()
    vis.update_renderer()

    if do_save:
        vis.capture_screen_image(os.path.join(dest_folder, '{:04d}.png'.format(c)))
    c += 1

vis.destroy_window()










