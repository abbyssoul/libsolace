/*
*  Copyright 2016 Ivan Ryabov
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/
/*******************************************************************************
 * @file: io/serial.cpp
 *
 *  Created by soultaker on 27/04/16.
*******************************************************************************/
#include <solace/io/serial.hpp>
#include <solace/io/platformFilesystem.hpp>
#include <solace/exception.hpp>


#include <tuple>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>


using namespace Solace;


const StringLiteral HW_ID_NA = "n/a";



String readLine(const Path& file) {
    const auto& pathString = file.toString();
    std::ifstream ifs(pathString.to_str(), std::ifstream::in);

    std::string line;
    if (ifs) {
        getline(ifs, line);
    }

    return String{ std::move(line) };
}


String usb_sysfs_friendly_name(const Path& sys_usb_path) {
    const auto manufacturer = readLine(sys_usb_path.join("manufacturer"));
    const auto product = readLine(sys_usb_path.join("product"));
    const auto serial = readLine(sys_usb_path.join("serial"));

    if (manufacturer.empty() && product.empty() && serial.empty())
        return String::Empty;

    return String::join(" ", {manufacturer, product, serial});
}


String usb_sysfs_hw_string(const Path& sysfs_path) {
    const auto vid = readLine(sysfs_path.join("idVendor"));
    const auto pid = readLine(sysfs_path.join("idProduct"));
    const auto serial_number = readLine(sysfs_path.join("serial"));

    return String{"USB VID:PID="}
            .concat(String::join(":", {vid, pid}))
            .concat(serial_number.empty()
                    ? serial_number
                    : String(" SNR=").concat(serial_number));
}


std::tuple<String, String> get_sysfs_info(IO::PlatformFilesystem const& fs, Path const& devicePath) {
    static const Path SYS_TTY_PATH = Path::Root.join({StringView{"sys"},
                                                      StringView{"class"},
                                                      StringView{"tty"}});

    String friendly_name;
    String hardware_id;

    const auto device_name = devicePath.getBasename();
    const auto sys_device_path = SYS_TTY_PATH.join(device_name).join("device");

    if (device_name.startsWith("ttyUSB")) {
        const auto deviceSysPath = fs.realPath(sys_device_path).getParent().getParent();

        if (fs.exists(deviceSysPath)){
            friendly_name = usb_sysfs_friendly_name(deviceSysPath);
            hardware_id = usb_sysfs_hw_string(deviceSysPath);
        }
    } else if (device_name.startsWith("ttyACM")) {
        const auto deviceSysPath = fs.realPath(sys_device_path).getParent();

        if (fs.exists(deviceSysPath)) {
            friendly_name = usb_sysfs_friendly_name(deviceSysPath);
            hardware_id = usb_sysfs_hw_string(deviceSysPath);
        }
    } else {
        // Try to read ID string of PCI device
        const auto sys_id_path = sys_device_path.join("id");

        if (fs.exists(sys_id_path)) {
            hardware_id = readLine(sys_id_path);
        }
    }

    if (friendly_name.empty())
        friendly_name = device_name;

    if (hardware_id.empty())
        hardware_id = HW_ID_NA;

    return std::make_tuple(friendly_name, hardware_id);
}


Solace::Array<Solace::IO::SerialPortInfo>
Solace::IO::Serial::enumeratePorts() {

    auto fs = PlatformFilesystem();
    const auto devices_found = fs.glob({"/dev/ttyACM*",
                                        "/dev/ttyS*",
                                        "/dev/ttyUSB*",
                                        "/dev/tty.*",
                                        "/dev/cu.*"
                                        });

    // NOTE: The vector is used here even thou the size is know already because we don't want to initialize structs yet
    std::vector<Solace::IO::SerialPortInfo> results;
    results.reserve(devices_found.size());

    for (const auto& device : devices_found) {
        String friendly_name;
        String hardware_id;
        std::tie(friendly_name, hardware_id) = get_sysfs_info(fs, device);

        results.emplace_back(device, friendly_name, hardware_id);
    }

    return { results };
}
