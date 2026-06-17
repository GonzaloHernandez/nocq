# benchmark_data.py

path = "/home/chalo/games/pgsolver"

files_small = [
    'cliquegame-100.gm',
    'cliquegame-200.gm',
    'cliquegame-500.gm',
    'cliquegame-1000.gm',
    'elevatorverification-3.gm',
    'elevatorverification-u-3.gm',
    'jurd-10-10.gm',
    'jurd-10-20.gm',
    'jurd-20-10.gm',
    'jurd-10-30.gm',
    'jurd-30-10.gm',
    'jurd-20-20.gm',
    'jurd-30-20.gm',
    'jurd-20-30.gm',
    'jurd-30-30.gm',
    'laddergame-100.gm',
    'laddergame-200.gm',
    'laddergame-500.gm',
    'steadygame-1000-1-20-1-20-11.gm',
    'steadygame-1000-1-20-1-20-12.gm',
    'steadygame-1000-1-20-1-20-13.gm',
    'steadygame-1000-1-20-1-20-14.gm',
    'towersofhanoi-5.gm'
]

inits_small = [
    17,17,73,351,460,279,662,1789,555,18,
    2518,602,1397,587,507,12,286,331,15,847,
    997,546,190
]

thenergy_small = [
    62, -12, -46, 78, 93, -9, 45, -53, -93, 17,
    66, 98, 38, -52, -91, 33, -94, 53, 9, -99,
    28, 55, -96    
]

thmean_small = [
    -43, 38, -56, 64, -98, 71, -40, -15, 30, 44,
    -24, -75, 89, -45, 51, 72, -22, -83, -48, -67,
    -10, 37, -57
]

files_large = [
    'cliquegame-2000.gm',
    'cliquegame-5000.gm',
    'cliquegame-10000.gm',
    'elevatorverification-4.gm',
    'elevatorverification-u-4.gm',
    'elevatorverification-5.gm',
    'elevatorverification-u-5.gm',
    'elevatorverification-6.gm',
    'elevatorverification-u-6.gm',
    'elevatorverification-7.gm',
    'elevatorverification-u-7.gm',
    'jurd-50-50.gm',
    'jurd-50-100.gm',
    'jurd-100-50.gm',
    'jurd-50-200.gm',
    'jurd-100-100.gm',
    'jurd-200-50.gm',
    'jurd-100-200.gm',
    'jurd-200-100.gm',
    'jurd-50-500.gm',
    'jurd-500-50.gm',
    'jurd-200-200.gm',
    'jurd-100-500.gm',
    'jurd-500-100.gm',
    'jurd-200-500.gm',
    'jurd-500-200.gm',
    'jurd-500-500.gm',
    'laddergame-1000.gm',
    'laddergame-2000.gm',
    'laddergame-5000.gm',
    'laddergame-10000.gm',
    'laddergame-20000.gm',
    'laddergame-50000.gm',
    'steadygame-5000-1-20-1-20-11.gm',
    'steadygame-5000-1-20-1-20-12.gm',
    'steadygame-5000-1-20-1-20-13.gm',
    'steadygame-5000-1-20-1-20-14.gm',
    'steadygame-10000-1-20-1-20-11.gm',
    'steadygame-10000-1-20-1-20-12.gm',
    'steadygame-10000-1-20-1-20-13.gm',
    'steadygame-10000-1-20-1-20-14.gm',
    'steadygame-20000-1-20-1-20-11.gm',
    'steadygame-20000-1-20-1-20-12.gm',
    'steadygame-20000-1-20-1-20-13.gm',
    'steadygame-20000-1-20-1-20-14.gm',
    'steadygame-50000-1-20-1-20-11.gm',
    'steadygame-50000-1-20-1-20-12.gm',
    'steadygame-50000-1-20-1-20-13.gm',
    'steadygame-50000-1-20-1-20-14.gm',
    'towersofhanoi-6.gm',
    'towersofhanoi-7.gm',
    'towersofhanoi-8.gm',
    'towersofhanoi-9.gm',
    'towersofhanoi-10.gm',
    'towersofhanoi-11.gm'
]

inits_large = [
    452,3609,7993,1337,680,1260,14127,27325,68472,750292,
    839721,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,1530,610,6580,7339,5702,70604,4166,636,
    2034,3625,9047,5024,9576,6999,10375,7245,18269,12184,
    26433,14347,28860,45214,2656,1309,21119,11177,22622
]

thenergy_large = [
    78, -16, -90, -75, 27, 94, 44, 11, -56, -9,
    -79, -38, 86, -34, 84, -69, -72, -64, -35, -67,
    -94, 25, -59, 25, -4, -51, -93, -27, 7, 18,
    74, -15, -18, -67, 37, -27, -22, -64, 79, -36,
    -19, 41, 88, -98, 45, -93, -91, 36, 34, 56,
    53, 12, -73, -86, 45
]

thmean_large = [
    -70, 70, 91, -54, -2, -82, 30, -2, 89, -93,
    6, -13, -54, -77, 8, 31, -58, -78, 89, 10,
    39, -43, -38, 28, 48, -31, -84, 68, -21, -49,
    24, -21, 60, -14, -99, 15, -34, -3, 18, 75,
    -86, 67, 1, 9, 3, -40, 74, -68, -98, 97,
    84, -89, 87, 92, 64
]