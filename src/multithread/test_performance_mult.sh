for n in 1 2 4 8 16; do
    for ((i=1; i<=10; i++)); do
    	echo 
    	./saxpy_multithread -n $n
    done
done
