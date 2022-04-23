for i in $(seq 1 150)
do
random=$(od -An -N2 -s < /dev/random)
echo $random
done