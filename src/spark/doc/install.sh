echo 'Spark driver is installing'

sudo cp ./rules/70-ttyusb.rules /etc/udev/rules.d/
sudo cp ./rules/orbbec-usb.rules /etc/udev/rules.d/556-orbbec-usb.rules

sudo apt-get install ros-indigo-ecl ros-indigo-ecl-threads ros-indigo-rgbd-launch
sudo apt-get install ros-indigo-image-common

echo 'Spark driver is installed'

