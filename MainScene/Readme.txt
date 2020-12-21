Creating this new scene due to the fact I had to change my pc and my previus scene stopped working. I assume its a driver problem.



To execute the code you will need to:
-Download all the files in the same directory;

- apt install libglew-dev libglfw3-dev libxi-dev;

- in a temporary directory type $git clone https://github.com/nothings/stb.git && sudo cp stb/stb_image.h /usr/local/include/ 

-and $git clone git@github.com:g-truc/glm.git && sudo cp -r glm/glm /usr/local/include/glm && sudo chown -R $USER /usr/local/include/glm

after that you can just use "make run" to compile the scene
