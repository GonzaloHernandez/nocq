# benchmark_data.py
import os
path = os.path.abspath("games/modelchecking")

files_small = [
    'Lift_(Correct)nlifts=4_liveness_1_1.gm',
    'Lift_(Correct)nlifts=4_liveness_1_2.gm',
    'SWPdatasize=2_windowsize=2_infinitely_often_enabled_then_infinitely_often_taken.gm',
    'Lift_(Incorrect)nlifts=4_liveness_2_1.gm',
    'Lift_(Incorrect)nlifts=4_liveness_2_2.gm',
    'SWPdatasize=4_windowsize=2_nodeadlock.gm',
    'SWPdatasize=8_windowsize=1_infinitely_often_enabled_then_infinitely_often_taken.gm',
    'Onebitdatasize=2_message_can_be_lost_infinitely_often.gm',
    'Onebitdatasize=2_messages_read_are_inevitably_sent.gm',
    'SWPdatasize=4_windowsize=2_infinitely_often_receive_d1.gm',
    'Onebitdatasize=2_messages_can_be_overtaken.gm',
    'Onebitdatasize=2_infinitely_often_read_write.gm',
    'Hanoindisks=11_eventually_done.gm',
    'Onebitdatasize=2_infinitely_often_receive_for_all_d.gm',
    'Onebitdatasize=2_no_spontaneous_messages.gm',
    'IEEE1394nparties=2_datasize=2_headersize=2_acksize=2_nodeadlock.gm',
    'SWPdatasize=4_windowsize=2_no_generation_of_messages.gm',
    'SWPdatasize=2_windowsize=3_nodeadlock.gm',
    'Onebitdatasize=2_invariantly_infinitely_many_reachable_taus.gm',
    'Onebitdatasize=2_no_duplication_of_messages.gm',
    'Lift_(Incorrect)nlifts=4_safety_2_1.gm',
    'Lift_(Incorrect)nlifts=4_safety_2_2.gm',
    'Onebitdatasize=3_nodeadlock.gm',
    'SWPdatasize=2_windowsize=3_infinitely_often_receive_d1.gm',
    'Onebitdatasize=2_read_then_eventually_send.gm',
    'Onebitdatasize=3_infinitely_often_receive_d1.gm',
    'SWPdatasize=4_windowsize=2_infinitely_often_read_write.gm',
    'IEEE1394nparties=2_datasize=2_headersize=2_acksize=2_property1.gm',
    'SWPdatasize=4_windowsize=2_read_then_eventually_send_if_fair.gm',
    'IEEE1394nparties=2_datasize=2_headersize=2_acksize=2_property2.gm',
    'SWPdatasize=4_windowsize=2_invariantly_infinitely_many_reachable_taus.gm',
    'Onebitdatasize=3_no_generation_of_messages.gm',
    'SWPdatasize=4_windowsize=2_infinitely_often_lost.gm',
    'Hanoindisks=12_eventually_done.gm',
    'Hesselinkdatasize=2_nodeadlock.gm',
    'Clobberwidth=4_height=4_black_has_winning_strategy.gm',
    'Clobberwidth=4_height=4_white_has_winning_strategy.gm',
    'IEEE1394nparties=2_datasize=2_headersize=2_acksize=2_property4.gm',
    'SWPdatasize=2_windowsize=3_infinitely_often_read_write.gm',
    'Onebitdatasize=3_message_can_be_lost_infinitely_often.gm',
    'Onebitdatasize=3_messages_read_are_inevitably_sent.gm',
    'SWPdatasize=2_windowsize=3_read_then_eventually_send_if_fair.gm',
    'SWPdatasize=2_windowsize=3_infinitely_often_receive_for_all_d.gm',
    'Onebitdatasize=3_infinitely_often_read_write.gm',
    'Onebitdatasize=3_messages_can_be_overtaken.gm',
    'SWPdatasize=4_windowsize=2_infinitely_often_receive_for_all_d.gm',
    'Onebitdatasize=2_infinitely_often_enabled_then_infinitely_often_taken.gm',
    'SWPdatasize=2_windowsize=3_invariantly_infinitely_many_reachable_taus.gm',
    'SWPdatasize=2_windowsize=3_infinitely_often_lost.gm',
    'Lift_(Incorrect)nlifts=4_safety_1.gm',
    'SWPdatasize=4_windowsize=2_no_duplication_of_messages.gm',
    'Onebitdatasize=3_invariantly_infinitely_many_reachable_taus.gm',
    'SWPdatasize=4_windowsize=2_read_then_eventually_send.gm',
    'SWPdatasize=2_windowsize=3_read_then_eventually_send.gm',
    'Onebitdatasize=3_infinitely_often_receive_for_all_d.gm',
    'SWPdatasize=2_windowsize=3_no_duplication_of_messages.gm',
    'Lift_(Incorrect)nlifts=4_nodeadlock.gm'
]

nvertices_small = [
    107275, 107275, 112513, 134161, 134161,
    140352, 147457, 153984, 153984, 163393,
    164352, 170752, 177148, 177667, 185088,
    188569, 211954, 223392, 245760, 258049,
    267377, 267377, 289296, 294433, 297088,
    308737, 353088, 377112, 388417, 417015,
    421056, 474913, 512017, 531442, 540736,
    564913, 564913, 571377, 576384, 579744,
    579744, 586657, 588867, 607752, 638064,
    653573, 655361, 670176, 742969, 788878,
    858113, 867888, 869568, 917712, 926212,
    944089, 998789
]

inits_small = [
    60479,67770,55612,99327,54219,72264,3299,15040,34835,27788,
    72486,9172,25335,42373,41217,49888,75568,4065,104137,58649,
    86941,8224,100890,41814,9911,35320,7546,87715,66072,80482,
    66110,61872,59923,51586,65401,96019,37330,17502,77997,54531,
    46586,104825,98920,460,19086,70478,54936,9588,55142,1990,
    85592,80381,86255,79411,62560,89120,52068
]

thenergy_small = [
    28, 98, -48, 45, -87, 42, -87, 72, -77, -38,
    24, -83, -84, -32, -4, 39, 35, 49, 28, 79,
    -91, 50, 50, 28, -75, 62, -10, -69, 89, -24,
    -38, -5, 57, -99, -7, 31, -4, 97, -4, 24,
    61, 73, -85, 15, 63, -94, 88, 15, 22, 35,
    1, -30, -49, 13, -16, 83, -12
]

thmean_small = [
    -41, -95, -12, 91, -29, -39, -14, 27, 92, -97,
    -9, 30, 23, 77, -99, 74, 41, 82, 73, -61,
    80, 8, 66, -37, 16, -47, -18, -29, 56, 74,
    17, -79, 32, 4, 15, 80, 95, -86, 12, 90,
    -78, 76, -1, 94, -17, -15, 25, 59, 12, 35,
    7, 19, 27, -81, 98, 98, -49    
]

files_large = [
    'Hesselinkdatasize=2_property1.gm',
    'Hesselinkdatasize=2_property2.gm',
    'Onebitdatasize=3_no_duplication_of_messages.gm',
    'Onebitdatasize=3_no_spontaneous_messages.gm',
    'IEEE1394nparties=2_datasize=2_headersize=2_acksize=2_property3.gm',
    'Onebitdatasize=3_read_then_eventually_send.gm',
    'IEEE1394nparties=2_datasize=2_headersize=2_acksize=2_property5.gm',
    'Hanoindisks=13_eventually_done.gm',
    'SWPdatasize=2_windowsize=3_infinitely_often_enabled_then_infinitely_often_taken.gm',
    'Lift_(Incorrect)nlifts=4_liveness_1_1.gm',
    'Lift_(Incorrect)nlifts=4_liveness_1_2.gm',
    'BRPdatasize=2_counting.gm',
    'SWPdatasize=4_windowsize=2_infinitely_often_enabled_then_infinitely_often_taken.gm',
    'Leadernparticipants=4_eventually-stable.gm',
    'SWPdatasize=2_windowsize=4_nodeadlock.gm',
    'Onebitdatasize=3_infinitely_often_enabled_then_infinitely_often_taken.gm',
    'SWPdatasize=2_windowsize=4_infinitely_often_receive_d1.gm',
    'SWPdatasize=4_windowsize=3_no_generation_of_messages.gm',
    'SWPdatasize=2_windowsize=4_infinitely_often_read_write.gm',
    'SWPdatasize=2_windowsize=4_infinitely_often_receive_for_all_d.gm',
    'SWPdatasize=2_windowsize=4_read_then_eventually_send_if_fair.gm',
    'SWPdatasize=4_windowsize=3_nodeadlock.gm',
    'SWPdatasize=2_windowsize=4_invariantly_infinitely_many_reachable_taus.gm',
    'SWPdatasize=4_windowsize=3_infinitely_often_receive_d1.gm',
    'SWPdatasize=2_windowsize=4_infinitely_often_lost.gm',
    'SWPdatasize=2_windowsize=4_read_then_eventually_send.gm',
    'SWPdatasize=2_windowsize=4_no_duplication_of_messages.gm',
    'Hesselinkdatasize=3_nodeadlock.gm',
    'SWPdatasize=4_windowsize=3_infinitely_often_read_write.gm',
    'SWPdatasize=2_windowsize=4_infinitely_often_enabled_then_infinitely_often_taken.gm',
    'SWPdatasize=4_windowsize=3_invariantly_infinitely_many_reachable_taus.gm',
    'SWPdatasize=4_windowsize=3_read_then_eventually_send_if_fair.gm',
    'Hesselinkdatasize=3_property2.gm'
]

nvertices_large = [
    1081473, 1093760, 1191961, 1278432, 1285575,
    1350432, 1411273, 1594324, 1787137, 1997578,
    1997578, 2177201, 2245633, 2341445, 2589056,
    3471553, 3487361, 6690604, 6823296, 6974723,
    7310721, 7429632, 7767168, 8835073, 8964337,
    10782592, 11488273, 13834800, 19550208, 20712449,
    22288896, 24565249, 27876960
]

inits_large = [
    335715,916028,829757,146210,
    122119,663419,642216,163350,
    691335,603407,130618,145204,
    601320,344089,943688,61268,
    985922,1047376,741469,870461,
    946918,204980,436829,846501,
    954487,
    1917974,6521602,9348135,1279004,
    4171266,2716459,10011207,2027123
]

thenergy_large = [
    -11, -78, -58, -69, 44, 88, 66, -97, 23, -18,
    28, -12, 88, 14, 87, -77, 43, -14, 94, 43,
    -100, 76, 61, -38, -6, -93, 59, 98, -5, 21,
    -90, -100, -1
]

thmean_large = [
    34, 53, -88, 61, -79, 52, -72, 9, 52, 12,
    96, 59, 75, 39, 14, 66, 59, 95, 52, -3,
    29, 25, -70, -94, -73, -64, 85, 81, 52, -80,
    3, 56, 95
]