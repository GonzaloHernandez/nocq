# benchmark_data.py

path = "/home/chalo/games/equivchecking"

# files = [
#     'CABP_SWP_(datasize=4_capacity=1_windowsize=1)eq=strong-bisim.gm',
#     'Par_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'Par_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'ABP(BW)_SWP_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm',
#     'ABP_CABP_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm',
#     'ABP(BW)_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'ABP(BW)_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'ABP_CABP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'ABP_CABP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'CABP_Par_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm',
#     'ABP(BW)_CABP_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm',
#     'CABP_Par_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'CABP_Par_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'ABP(BW)_CABP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'ABP(BW)_CABP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'SWP_SWP_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm',
#     'SWP_SWP_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'SWP_SWP_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'CABP_SWP_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm',
#     'CABP_SWP_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'CABP_SWP_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'SWP_SWP_(datasize=3_capacity=1_windowsize=1)eq=weak-bisim.gm',
#     'SWP_SWP_(datasize=3_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'SWP_SWP_(datasize=3_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'Buffer_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'Buffer_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'Buffer_Onebit_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm',
#     'CABP_SWP_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm',
#     'CABP_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm',
#     'CABP_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm',
#     'Buffer_Onebit_(datasize=2_capacity=2_windowsize=1)eq=branching-bisim.gm',
#     'Buffer_Onebit_(datasize=2_capacity=2_windowsize=1)eq=branching-sim.gm',
#     'Buffer_Onebit_(datasize=2_capacity=2_windowsize=1)eq=weak-bisim.gm'
# ]

# inits = [
#     81906, 102043, 29198, 55522, 75186, 50561, 2002, 66099, 82996, 61050, 14849, 53799,
#     55796, 54531, 98617, 4511, 80581, 49632, 76077, 58236, 61216, 61744, 37535, 86785,
#     98342, 43854, 840, 26835, 25240, 99256, 50992, 79542, 11644
# ]

files = [
    'Buffer_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-bisim.gm',
    'Buffer_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-sim.gm',
    'Buffer_Onebit_(datasize=3_capacity=1_windowsize=1)eq=weak-bisim.gm',
    'Buffer_Onebit_(datasize=3_capacity=2_windowsize=1)eq=branching-bisim.gm',
    'Buffer_Onebit_(datasize=3_capacity=2_windowsize=1)eq=branching-sim.gm',
    'Buffer_Onebit_(datasize=3_capacity=2_windowsize=1)eq=weak-bisim.gm',
    'SWP_SWP_(datasize=2_capacity=1_windowsize=2)eq=strong-bisim.gm',
    'Onebit_SWP_(datasize=2_capacity=1_windowsize=1)eq=strong-bisim.gm',
    'CABP_Onebit_(datasize=2_capacity=1_windowsize=1)eq=strong-bisim.gm',
    'ABP_Onebit_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm',
    'ABP_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm',
    'ABP_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm',
    'Par_Onebit_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm',
    'ABP(BW)_Onebit_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm',
    'Par_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm',
    'Par_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm',
    'ABP(BW)_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm',
    'ABP(BW)_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm',
    'SWP_SWP_(datasize=3_capacity=1_windowsize=2)eq=strong-bisim.gm',
    'Onebit_SWP_(datasize=3_capacity=1_windowsize=1)eq=strong-bisim.gm',
    'CABP_Onebit_(datasize=3_capacity=1_windowsize=1)eq=strong-bisim.gm',
    'ABP_Onebit_(datasize=3_capacity=1_windowsize=1)eq=weak-bisim.gm',
    'ABP_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-bisim.gm',
    'ABP_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-sim.gm',
    'SWP_SWP_(datasize=2_capacity=1_windowsize=2)eq=weak-bisim.gm',
    'ABP(BW)_Onebit_(datasize=3_capacity=1_windowsize=1)eq=weak-bisim.gm',
    'SWP_SWP_(datasize=2_capacity=1_windowsize=2)eq=branching-bisim.gm',
    'SWP_SWP_(datasize=2_capacity=1_windowsize=2)eq=branching-sim.gm',
    'ABP(BW)_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-bisim.gm',
    'ABP(BW)_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-sim.gm',
    'Hesselink_(Implementation)_Hesselink_(Specification)_(datasize=2)eq=weak-bisim.gm',
    'Hesselink_(Specification)_Hesselink_(Implementation)_(datasize=2)eq=weak-bisim.gm',
    'Hesselink_(Implementation)_Hesselink_(Specification)_(datasize=2)eq=branching-bisim.gm',
    'Hesselink_(Implementation)_Hesselink_(Specification)_(datasize=2)eq=branching-sim.gm',
    'Hesselink_(Specification)_Hesselink_(Implementation)_(datasize=2)eq=branching-bisim.gm',
    'Hesselink_(Specification)_Hesselink_(Implementation)_(datasize=2)eq=branching-sim.gm'
]

inits = [
    1899212,1005347,25342,1108320,
    585804,1794680,104100,430147,
    1488761,850480,1691030,1022194,
    515643,
    1521621,2404963,4584758,9706440,
    3754954,3125639,3089031,10409006,
    4966629,5034823,2372023,6309757,
    4988428,1584363,2173080,10098704,
    10209881,
    10317801,15795522,23044171,28502312,
    11677260,1705429
]