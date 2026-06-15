path="benchmarks"
# Games
files=(
     'CABP_SWP_(datasize=4_capacity=1_windowsize=1)eq=strong-bisim.gm'
     'Par_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'Par_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm'
     'ABP(BW)_SWP_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm'
     'ABP_CABP_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm'
     'ABP(BW)_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'ABP(BW)_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm'
     'ABP_CABP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'ABP_CABP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm'
     'CABP_Par_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm'
     'ABP(BW)_CABP_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm'
     'CABP_Par_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'CABP_Par_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm'
     'ABP(BW)_CABP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'ABP(BW)_CABP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm'
     'SWP_SWP_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm'
     'SWP_SWP_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'SWP_SWP_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm'
     'CABP_SWP_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm'
     'CABP_SWP_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'CABP_SWP_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm'
     'SWP_SWP_(datasize=3_capacity=1_windowsize=1)eq=weak-bisim.gm'
     'SWP_SWP_(datasize=3_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'SWP_SWP_(datasize=3_capacity=1_windowsize=1)eq=branching-sim.gm'
     'Buffer_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'Buffer_Onebit_(datasize=2_capacity=1_windowsize=1)eq=branching-sim.gm'
     'Buffer_Onebit_(datasize=2_capacity=1_windowsize=1)eq=weak-bisim.gm'
     'CABP_SWP_(datasize=4_capacity=1_windowsize=1)eq=weak-bisim.gm'
     'CABP_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-bisim.gm'
     'CABP_SWP_(datasize=4_capacity=1_windowsize=1)eq=branching-sim.gm'
     'Buffer_Onebit_(datasize=2_capacity=2_windowsize=1)eq=branching-bisim.gm'
     'Buffer_Onebit_(datasize=2_capacity=2_windowsize=1)eq=branching-sim.gm'
     'Buffer_Onebit_(datasize=2_capacity=2_windowsize=1)eq=weak-bisim.gm'
)

# Number of vertices of the game
sizes=(
    104073 107823 107823 109889 118224 124457 124457 134096 134096 147322 
    156728 167246 167246 178232 178232 214896 240096 240096 245696 276224
    276224 440568 488628 488628 604353 604353 631473 700288 788224 788224
    912640 912640
)

# Random init vertices
inits=(
    81906 102043 29198 55522 75186 50561 2002 66099 82996 61050 14849 53799
    55796 54531 98617 4511 80581 49632 76077 58236 61216 61744 37535 86785
    98342 43854 840 26835 25240 99256 50992 79542 11644
)

thresholdEnergys=(
    23 -39 -50 -8 -74 93 -97 -81 73 -12 -41 -78 89 67 11 16 -91 30 -99 59
    -3 -47 19 -27 -70 70 4 -66 13 -66 -66 -87 -67 
)

thresholdMeans=(
    -60 19 19 87 -20 0 -95 1 -32 -14 -41 -20 -19 -7 -52 -6 -5 -97 -2 10
    22 -23 77 55 99 99 42 59 93 -16 30 -9 30
)

csv_file="01_small_results_equivalncech.csv"
csv_log="01_small_raw_equivalncech.csv"
n=3

# Write the CSV Header line at the very beginning
echo "time,solved" > "$csv_file"
echo "Oink(PP),Oink(PP+),Oink(PAR),ZRA,NOCQ" > "$csv_log"

# Print status header to terminal
echo "------------------------------------------------" >&2
echo "Starting benchmarks... Output saving to $csv_file" >&2
echo "------------------------------------------------" >&2

# Initialize our statistical tracking variables
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
    docker run --rm -v "$(pwd)/benchmarks":/mnt solver oink "/mnt/$file" --pp > /tmp/oink_$i.txt 2>&1 &
    pid_oink=$!

    sleep 60 &
    pid_timeout_oink=$!
    
    wait -n
    kill $pid_oink $pid_timeout_oink 2>/dev/null
    wait $pid_oink $pid_timeout_oink 2>/dev/null 

    r_oink=$(grep "total solving time:" /tmp/oink_$i.txt 2>/dev/null | awk '{print $6}')
    rm -f /tmp/oink_$i.txt

    if [ -n "$r_oink" ]; then
        parity_result="$r_oink"
    else
        parity_result="TIMEOUT"
        echo "   --> Warning: Output unknown or Time out!" >&2
    fi

    # Update Statistics
    if [ "$parity_result" != "TIMEOUT" ]; then
        oink_pp_count=$((oink_pp_count + 1))
        oink_pp_time=$(awk "BEGIN {print $oink_pp_time + $parity_result}")
    else
        oink_pp_time=$(awk "BEGIN {print $oink_pp_time + 120}")
    fi

    printf "%s," "$parity_result" >> "$csv_log"
 
    # ====================================================================
    # OINK Priority Promotion (+) SECTION
    # ====================================================================
    docker run --rm -v "$(pwd)/benchmarks":/mnt solver oink "/mnt/$file" --ppp > /tmp/oink_$i.txt 2>&1 &
    pid_oink=$!

    sleep 60 &
    pid_timeout_oink=$!
    
    wait -n
    kill $pid_oink $pid_timeout_oink 2>/dev/null
    wait $pid_oink $pid_timeout_oink 2>/dev/null 

    r_oink=$(grep "total solving time:" /tmp/oink_$i.txt 2>/dev/null | awk '{print $6}')
    rm -f /tmp/oink_$i.txt

    if [ -n "$r_oink" ]; then
        parity_result="$r_oink"
    else
        parity_result="TIMEOUT"
        echo "   --> Warning: Output unknown or Time out!" >&2
    fi

    # Update Statistics
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
    docker run --rm -v "$(pwd)/benchmarks":/mnt solver oink "/mnt/$file" --zlkpp-std > /tmp/oink_$i.txt 2>&1 &
    pid_oink=$!

    sleep 60 &
    pid_timeout_oink=$!
    
    wait -n
    kill $pid_oink $pid_timeout_oink 2>/dev/null
    wait $pid_oink $pid_timeout_oink 2>/dev/null 

    r_oink=$(grep "total solving time:" /tmp/oink_$i.txt 2>/dev/null | awk '{print $6}')
    rm -f /tmp/oink_$i.txt

    if [ -n "$r_oink" ]; then
        parity_result="$r_oink"
    else
        parity_result="TIMEOUT"
        echo "   --> Warning: Output unknown or Time out!" >&2
    fi

    # Update Statistics
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
    docker run --rm -v "$(pwd)/benchmarks":/mnt nocq --gm "/mnt/$file" --zra --print-only-time --init "$init" > /tmp/parity_$i.txt 2>/dev/null &
    pid_zra=$!

    sleep 60 &
    pid_timeout_zra=$!
    
    # Wait until either the solver or the sleep timer exits
    wait -n
    kill $pid_zra $pid_timeout_zra 2>/dev/null
    wait $pid_zra $pid_timeout_zra 2>/dev/null 

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
    docker run --rm -v "$(pwd)/benchmarks":/mnt nocq --gm "/mnt/$file" --noc-even --parity --print-only-time --init "$init" > /tmp/parity_even_$i.txt 2>/dev/null &
    pid_even=$!
    
    docker run --rm -v "$(pwd)/benchmarks":/mnt nocq --gm "/mnt/$file" --noc-odd --parity --print-only-time --init "$init" > /tmp/parity_odd_$i.txt 2>/dev/null &
    pid_odd=$!

    sleep 60 &
    pid_timeout_nocq=$!

    # Wait for the first of the three parallel paths to complete
    wait -n
    kill $pid_even $pid_odd $pid_timeout_nocq 2>/dev/null
    wait $pid_even $pid_odd $pid_timeout_nocq 2>/dev/null 

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

    # Update Statistics
    if [ "$parity_result" != "TIMEOUT" ]; then
        nocq_count=$((nocq_count + 1))
        nocq_time=$(awk "BEGIN {print $nocq_time + $parity_result}")
    else
        nocq_time=$(awk "BEGIN {print $nocq_time + 120}") # Added closing bracket }
    fi

    # Log results on the same row, then append newline
    echo "$parity_result" >> "$csv_log"

done

# ====================================================================
# CALCULATE AND PRINT SUMMARY STATISTICS
# ====================================================================
echo "------------------------------------------------" >&2
echo "Done! All results written to $csv_file" >&2
echo "The raw data written to $csv_log" >&2
echo "------------------------------------------------" >&2

# ZRA Summary Output

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

# NOCQ Summary Output
if [ $nocq_count -gt 0 ]; then
    avg_time=$(awk "BEGIN {print $nocq_time / $nocq_count}")
    echo "NOCQ,${avg_time},${nocq_count}/${n}" >> "$csv_file"
else
    echo "NOCQ,$((n * 120)),${nocq_count}/${n}" >> "$csv_file"
fi