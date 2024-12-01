./cleanup.sh
find . -type f -regex '.*\.(py|cpp|h|i|txt|)$' -not -name 'all.txt' -exec awk 'FNR==1{print "\n//" FILENAME "\n"} {print}' {} + > all.txt
