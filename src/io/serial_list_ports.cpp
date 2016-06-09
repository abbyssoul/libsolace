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
#include <solace/exception.hpp>

#include <tuple>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>

#include <sys/stat.h>
#include <glob.h>


using Solace::String;
using Solace::Path;


const Path SYS_TTY_PATH = Path::parse("/sys/class/tty/");


// TODO(abbyssoul): move to Filesystem when its ready
std::vector<Path> glob(std::initializer_list<const char*> patterns) {
    std::vector<Path> paths_found;

    if (patterns.size() == 0)
        return paths_found;

    glob_t glob_results;
    glob(*(patterns.begin()), 0, NULL, &glob_results);

    auto iter = patterns.begin();
    while (++iter != patterns.end()) {
        glob(*iter, GLOB_APPEND, NULL, &glob_results);
    }

    for (size_t path_index = 0; path_index < glob_results.gl_pathc; path_index++) {
        paths_found.push_back(Path::parse(glob_results.gl_pathv[path_index]));
    }

    globfree(&glob_results);

    return paths_found;
}

// TODO(abbyssoul): move to Filesystem when its ready
bool path_exists(const Path& path) {
    struct stat sb;

    const auto& pathString = path.toString();
    return (stat(pathString.c_str(), &sb ) == 0);
}

Path realpath(const Path& path) {
    const auto& pathString = path.toString();
    std::unique_ptr<char, decltype(std::free)*> real_path{realpath(pathString.c_str(), NULL), std::free };

    return (real_path)
            ? Path::parse(String(real_path.get()))
            : Path::Root;
}


String read_line(const Path& file) {
    const auto& pathString = file.toString();
    std::ifstream ifs(pathString.to_str(), std::ifstream::in);

    std::string line;
    if (ifs) {
        getline(ifs, line);
    }

    return line;
}


String usb_sysfs_friendly_name(const Path& sys_usb_path) {
    const String manufacturer = read_line(sys_usb_path.join("manufacturer"));
    const String product = read_line(sys_usb_path.join("product"));
    const String serial = read_line(sys_usb_path.join("serial"));

    if (manufacturer.empty() && product.empty() && serial.empty())
        return String::Empty;

    return String::join(" ", {manufacturer, product, serial});
}


String usb_sysfs_hw_string(const Path& sysfs_path) {
    const auto vid = read_line(sysfs_path.join("idVendor"));
    const auto pid = read_line(sysfs_path.join("idProduct"));
    const auto serial_number = read_line(sysfs_path.join("serial"));

    return String("USB VID:PID=")
            .concat(String::join(":", {vid, pid}))
            .concat(serial_number.empty()
                    ? String(" SNR=").concat(serial_number)
                    : serial_number);
}


std::tuple<String, String> get_sysfs_info(const Path& devicePath) {
    String friendly_name;
    String hardware_id;

    const auto device_name = devicePath.getBasename();
    const auto sys_device_path = SYS_TTY_PATH.join(device_name).join("device");

    if (device_name.startsWith("ttyUSB")) {
        const auto deviceSysPath = realpath(sys_device_path).getParent().getParent();

        if (path_exists(deviceSysPath)){
            friendly_name = usb_sysfs_friendly_name(deviceSysPath);
            hardware_id = usb_sysfs_hw_string(deviceSysPath);
        }
    } else if (device_name.startsWith("ttyACM")) {
        const auto deviceSysPath = realpath(sys_device_path).getParent();

        if (path_exists(deviceSysPath)) {
            friendly_name = usb_sysfs_friendly_name(deviceSysPath);
            hardware_id = usb_sysfs_hw_string(deviceSysPath);
        }
    } else {
        // Try to read ID string of PCI device
        const auto sys_id_path = sys_device_path.join("id");
        if (path_exists(sys_id_path))
            hardware_id = read_line(sys_id_path);
    }

    if (friendly_name.empty())
        friendly_name = device_name;

    if (hardware_id.empty())
        hardware_id = "n/a";

    return std::make_tuple(friendly_name, hardware_id);
}


Solace::Array<Solace::IO::SerialPortInfo>
Solace::IO::Serial::enumeratePorts() {
    const auto devices_found = glob({   "/dev/ttyACM*",
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
        std::tie(friendly_name, hardware_id) = get_sysfs_info(device);

        results.emplace_back(device, friendly_name, hardware_id);
    }

    return { results };
}
