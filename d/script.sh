make
echo
echo Introduce the name of the file
read namefile
echo
echo Introduce number of executions
read napps

echo
echo graph_football
./$namefile -i ../input/instances/graph_football.txt n_apps $napps
echo
echo graph_jazz
./$namefile -i ../input/instances/graph_jazz.txt n_apps $napps
echo
echo ego-facebook
./$namefile -i ../input/instances/ego-facebook.txt n_apps $napps
echo
echo graph_actors_dat
./$namefile -i ../input/instances/graph_actors_dat.txt n_apps $napps
echo
echo graph_CA-AstroPh
./$namefile -i ../input/instances/graph_CA-AstroPh.txt n_apps $napps
echo
echo graph_CA-CondMat
./$namefile -i ../input/instances/graph_CA-CondMat.txt n_apps $napps
echo
echo graph_CA-HepPh
./$namefile -i ../input/instances/graph_CA-HepPh.txt n_apps $napps
echo
echo socfb-Brandeis99
./$namefile -i ../input/instances/socfb-Brandeis99.txt n_apps $napps
echo
echo socfb-Mich67
./$namefile -i ../input/instances/socfb-Mich67.txt n_apps $napps
echo
echo soc-gplus
./$namefile -i ../input/instances/soc-gplus.txt n_apps $napps
echo