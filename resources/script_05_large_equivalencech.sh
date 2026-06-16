path="/home/chalo/games/equivchecking"

files=(
    # equivchecking-parity-1000-9999k
    'Buffer_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-bisim.gm'
    'Buffer_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-sim.gm'
    'Buffer_Onebit_(datasize=3_capacity=1_windowsize=1)eq=weak-bisim.gm'
    'Buffer_Onebit_(datasize=3_capacity=2_windowsize=1)eq=branching-bisim.gm'
    'Buffer_Onebit_(datasize=3_capacity=2_windowsize=1)eq=branching-sim.gm'
    'Buffer_Onebit_(datasize=3_capacity=2_windowsize=1)eq=weak-bisim.gm'
    'SWP_SWP_(datasize=2_capacity=1_windowsize=2)eq=strong-bisim.gm'
    'Onebit_SWP_(datasize=2_capacity=1_windowsize=1)eq=strong-bisim.gm'
    'CABP_Onebit_(datasize=2_capacity=1_windowsize=1)eq=strong-bisim.gm'
    'ABP_Onebit_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm'
    'ABP_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm'
    'ABP_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm'
    'Par_Onebit_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm'

    # equivchecking-parity-10000-99999k
    'ABP(BW)_Onebit_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm'
    'Par_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm'
    'Par_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm'
    'ABP(BW)_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm'
    'ABP(BW)_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm'
    'SWP_SWP_(datasize=3_capacity=1_windowsize=2)eq=strong-bisim.gm'
    'Onebit_SWP_(datasize=3_capacity=1_windowsize=1)eq=strong-bisim.gm'
    'CABP_Onebit_(datasize=3_capacity=1_windowsize=1)eq=strong-bisim.gm'
    'ABP_Onebit_(datasize=3_capacity=1_windowsize=1)eq=weak-bisim.gm'
    'ABP_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-bisim.gm'
    'ABP_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-sim.gm'
    'SWP_SWP_(datasize=2_capacity=1_windowsize=2)eq=weak-bisim.gm'
    'ABP(BW)_Onebit_(datasize=3_capacity=1_windowsize=1)eq=weak-bisim.gm'
    'SWP_SWP_(datasize=2_capacity=1_windowsize=2)eq=branching-bisim.gm'
    'SWP_SWP_(datasize=2_capacity=1_windowsize=2)eq=branching-sim.gm'
    'ABP(BW)_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-bisim.gm'
    'ABP(BW)_Onebit_(datasize=3_capacity=1_windowsize=1)eq=branching-sim.gm'

    # equivchecking-hesselink-10000-99999k
    'Hesselink_(Implementation)_Hesselink_(Specification)_(datasize=2)eq=weak-bisim.gm'
    'Hesselink_(Specification)_Hesselink_(Implementation)_(datasize=2)eq=weak-bisim.gm'
    'Hesselink_(Implementation)_Hesselink_(Specification)_(datasize=2)eq=branching-bisim.gm'
    'Hesselink_(Implementation)_Hesselink_(Specification)_(datasize=2)eq=branching-sim.gm'
    'Hesselink_(Specification)_Hesselink_(Implementation)_(datasize=2)eq=branching-bisim.gm'
    'Hesselink_(Specification)_Hesselink_(Implementation)_(datasize=2)eq=branching-sim.gm'
)

# Random init vertices
inits=(
    1899212 1005347 25342 1108320 
    585804 1794680 104100 430147 
    1488761 850480 1691030 1022194 
    515643

    1521621 2404963 4584758 9706440 
    3754954 3125639 3089031 10409006 
    4966629 5034823 2372023 6309757 
    4988428 1584363 2173080 10098704 
    10209881

    10317801 15795522 23044171 28502312 
    11677260 1705429
)

csv_file="05_large_results_equivalncech.csv"
csv_log="05_large_raw_equivalncech.csv"
maxtime=75
n=36

echo "time,solved" > "$csv_file"
echo "Oink(PP),Oink(PP+),Oink(PAR),ZRA,NOCQ" > "$csv_log"

echo "------------------------------------------------" >&2
echo "Starting benchmarks... Output saving to $csv_file" >&2
echo "------------------------------------------------" >&2

oink_pp_count=0
oink_pp_time=0.0

oink_ppp_count=0
oink_ppp_time=0.0

oink_par_count=0
oink_par_time=0.0

zra_count=0
zra_time=0.0

nocq_count=0
nocq_time=0.0

for ((i=0; i<n; i++)); do
    file="${files[$i]}"
    size="${sizes[$i]}"
    init="${inits[$i]}"
    
    echo "Processing game $((i+1))/${n}: $file ..." >&2
    echo "Oink(PP)... Oink(PP+)... Oink(PAR)... ZRA... NOCQ..." >&2

    # ====================================================================
    # OINK Priority Promotion SECTION
    # ====================================================================
    timeout $maxtime docker run --rm -t --init -v ${path}:/mnt solver oink "/mnt/$file" --pp > /tmp/oink_$i.txt 2>&1 &
    pid_oink=$!

    wait $pid_oink 2>/dev/null

    r_oink=$(grep "total solving time:" /tmp/oink_$i.txt 2>/dev/null | awk '{print $6}')
    rm -f /tmp/oink_$i.txt

    if [ -n "$r_oink" ]; then
        parity_result="$r_oink"
    else
        parity_result="TIMEOUT"
        echo "   --> Warning: Output unknown or Time out!" >&2
    fi

    if [ "$parity_result" != "TIMEOUT" ]; then
        oink_pp_count=$((oink_pp_count + 1))
        oink_pp_time=$(awk "BEGIN {print $oink_pp_time + $parity_result}")
    else
        oink_pp_time=$(awk "BEGIN {print $oink_pp_time + 120}")
    fi

    printf "%s," "$parity_result" >> "$csv_log"
 
    # ====================================================================
    # OINK Priority Promotion(+) SECTION
    # ====================================================================
    timeout $maxtime docker run --rm -t --init -v "${path}":/mnt solver oink "/mnt/$file" --ppp > /tmp/oink_$i.txt 2>&1 &
    pid_oink=$!

    wait $pid_oink 2>/dev/null 

    r_oink=$(grep "total solving time:" /tmp/oink_$i.txt 2>/dev/null | awk '{print $6}')
    rm -f /tmp/oink_$i.txt

    if [ -n "$r_oink" ]; then
        parity_result="$r_oink"
    else
        parity_result="TIMEOUT"
        echo "   --> Warning: Output unknown or Time out!" >&2
    fi

    if [ "$parity_result" != "TIMEOUT" ]; then
        oink_ppp_count=$((oink_ppp_count + 1))
        oink_ppp_time=$(awk "BEGIN {print $oink_ppp_time + $parity_result}")
    else
        oink_ppp_time=$(awk "BEGIN {print $oink_ppp_time + 120}")
    fi

    printf "%s," "$parity_result" >> "$csv_log"
 
    # ====================================================================
    # OINK Parys' improvements SECTION
    # ====================================================================
    timeout $maxtime docker run --rm -t --init -v "${path}":/mnt solver oink "/mnt/$file" --zlkpp-std > /tmp/oink_$i.txt 2>&1 &
    pid_oink=$!
    wait $pid_oink 2>/dev/null 

    r_oink=$(grep "total solving time:" /tmp/oink_$i.txt 2>/dev/null | awk '{print $6}')
    rm -f /tmp/oink_$i.txt

    if [ -n "$r_oink" ]; then
        parity_result="$r_oink"
    else
        parity_result="TIMEOUT"
        echo "   --> Warning: Output unknown or Time out!" >&2
    fi

    if [ "$parity_result" != "TIMEOUT" ]; then
        oink_par_count=$((oink_par_count + 1))
        oink_par_time=$(awk "BEGIN {print $oink_par_time + $parity_result}")
    else
        oink_par_time=$(awk "BEGIN {print $oink_par_time + 120}")
    fi

    printf "%s," "$parity_result" >> "$csv_log"
    
    # ====================================================================
    # ZRA SECTION
    # ====================================================================
    timeout $maxtime docker run --rm -t --init -v "${path}":/mnt nocq --gm "/mnt/$file" --zra --print-only-time --init "$init" > /tmp/parity_$i.txt 2>/dev/null &
    pid_zra=$!

    wait $pid_zra 2>/dev/null

    r_zra=$(cat /tmp/parity_$i.txt 2>/dev/null | xargs)
    rm -f /tmp/parity_$i.txt

    if [ -n "$r_zra" ]; then
        parity_result="$r_zra"
    else
        parity_result="TIMEOUT"
        echo "   --> Warning: ZRA timed out!" >&2
    fi

    # Update Statistics
    if [ "$parity_result" != "TIMEOUT" ]; then
        zra_count=$((zra_count + 1))
        zra_time=$(awk "BEGIN {print $zra_time + $parity_result}")
    else
        zra_time=$(awk "BEGIN {print $zra_time + 120}") # Added closing bracket }
    fi

    printf "%s," "$parity_result" >> "$csv_log"

    # ====================================================================
    # NOCQ SECTION (Parallel EVEN vs ODD Race)
    # ====================================================================
    timeout $maxtime docker run --rm -t --init -v "${path}":/mnt nocq --gm "/mnt/$file" --noc-even --parity --print-only-time --init "$init" > /tmp/parity_even_$i.txt 2>/dev/null &
    pid_even=$!
    
    timeout $maxtime docker run --rm -t --init -v "${path}":/mnt nocq --gm "/mnt/$file" --noc-odd --parity --print-only-time --init "$init" > /tmp/parity_odd_$i.txt 2>/dev/null &
    pid_odd=$!

    wait -n
    kill $pid_even $pid_odd 2>/dev/null
    wait $pid_even $pid_odd 2>/dev/null 

    r_even=$(cat /tmp/parity_even_$i.txt 2>/dev/null)
    r_odd=$(cat /tmp/parity_odd_$i.txt 2>/dev/null)
    rm -f /tmp/parity_even_$i.txt /tmp/parity_odd_$i.txt
    
    if [ -n "$r_even" ]; then
        parity_result="$r_even"
    elif [ -n "$r_odd" ]; then
        parity_result="$r_odd"
    else
        parity_result="TIMEOUT"
        echo "   --> Warning: NOCQ timed out!" >&2
    fi

    if [ "$parity_result" != "TIMEOUT" ]; then
        nocq_count=$((nocq_count + 1))
        nocq_time=$(awk "BEGIN {print $nocq_time + $parity_result}")
    else
        nocq_time=$(awk "BEGIN {print $nocq_time + 120}") # Added closing bracket }
    fi

    echo "$parity_result" >> "$csv_log"

done

# ====================================================================
# CALCULATE AND PRINT SUMMARY STATISTICS
# ====================================================================
echo "------------------------------------------------" >&2
echo "Done! All results written to $csv_file" >&2
echo "The raw data written to $csv_log" >&2
echo "------------------------------------------------" >&2

if [ $oink_pp_count -gt 0 ]; then
    avg_time=$(awk "BEGIN {print $oink_pp_time / $oink_pp_count}")
    echo "Oink(PP),${avg_time},${oink_pp_count}/${n}" >> "$csv_file"
else
    echo "Oink(PP),$((n * 120)),${oink_pp_count}/${n}" >> "$csv_file"
fi

if [ $oink_ppp_count -gt 0 ]; then
    avg_time=$(awk "BEGIN {print $oink_ppp_time / $oink_ppp_count}")
    echo "Oink(PP+),${avg_time},${oink_ppp_count}/${n}" >> "$csv_file"
else
    echo "Oink(PP+),$((n * 120)),${oink_ppp_count}/${n}" >> "$csv_file"
fi

if [ $oink_par_count -gt 0 ]; then
    avg_time=$(awk "BEGIN {print $oink_par_time / $oink_par_count}")
    echo "Oink(PAR),${avg_time},${oink_par_count}/${n}" >> "$csv_file"
else
    echo "Oink(PAR),$((n * 120)),${oink_par_count}/${n}" >> "$csv_file"
fi

if [ $zra_count -gt 0 ]; then
    avg_time=$(awk "BEGIN {print $zra_time / $zra_count}")
    echo "ZRA,${avg_time},${zra_count}/${n}" >> "$csv_file"
else
    echo "ZRA,$((n * 120)),${zra_count}/${n}" >> "$csv_file"
fi

if [ $nocq_count -gt 0 ]; then
    avg_time=$(awk "BEGIN {print $nocq_time / $nocq_count}")
    echo "NOCQ,${avg_time},${nocq_count}/${n}" >> "$csv_file"
else
    echo "NOCQ,$((n * 120)),${nocq_count}/${n}" >> "$csv_file"
fi