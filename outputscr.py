from itertools import count
import os

foldername = "src"
src_dir = os.path.join(os.path.curdir, foldername)

outputfile = open('output.txt', 'w')
counter = 0

for subdir, dirs, files in os.walk(src_dir):
	for file in files:
		print('Current File: ', os.path.join(subdir, file))
		filepath = subdir + os.sep + file

		outputfile.write('Filename: ' + file + ':\n')
		outputfile.write(open(filepath).read())
		outputfile.write('\n')
		counter += 1

print('Files Read ', counter)