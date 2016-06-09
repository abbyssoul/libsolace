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
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fmt/format.h>


using std::vector;
using std::string;


vector<string> glob(std::initializer_list<const char*> patterns) {
    vector<string> paths_found;

    if (patterns.size() == 0)
        return paths_found;

    glob_t glob_results;
//    int glob_retval =
    glob(*(patterns.begin()), 0, NULL, &glob_results);

    std::initializer_list<const char*>::const_iterator iter = patterns.begin();
    while (++iter != patterns.end()) {
//        glob_retval =
        glob(*iter, GLOB_APPEND, NULL, &glob_results);
    }

    for (size_t path_index = 0; path_index < glob_results.gl_pathc; path_index++) {
        paths_found.push_back(glob_results.gl_pathv[path_index]);
    }

    globfree(&glob_results);

    return paths_found;
}


string basename(const string& path) {
    const auto pos = path.rfind("/");

    if (pos == std::string::npos)
        return path;

    return string(path, pos + 1, string::npos);
}


string dirname(const string& path) {
    const auto pos = path.rfind("/");

    if (pos == std::string::npos)
        return path;

    else if (pos == 0)
        return "/";

    return string(path, 0, pos);
}


bool path_exists(const string& path) {
    struct stat sb;

    return (stat(path.c_str(), &sb ) == 0);
}

string realpath(const string& path) {
    char* real_path = realpath(path.c_str(), NULL);

    string result;
    if (real_path != NULL) {
        result = real_path;
        free(real_path);
    }

    return result;
}


string read_line(const string& file) {
    std::ifstream ifs(file, std::ifstream::in);

    string line;
    if (ifs) {
        getline(ifs, line);
    }

    return line;
}


string usb_sysfs_friendly_name(const string& sys_usb_path) {
    uint device_number = 0;

    std::istringstream(read_line(sys_usb_path + "/devnum")) >> device_number;

    const string manufacturer = read_line(sys_usb_path + "/manufacturer");
    const string product = read_line(sys_usb_path + "/product");
    const string serial = read_line(sys_usb_path + "/serial");

    if (manufacturer.empty() && product.empty() && serial.empty())
        return "";

    // FIXME(abbyssoul): Should be String.join(" ", ...);
    return fmt::format("{} {} {}", manufacturer, product, serial);
}


string usb_sysfs_hw_string(const string& sysfs_path) {
    string serial_number = read_line(sysfs_path + "/serial");

    if (!serial_number.empty()) {
        serial_number = fmt::format("SNR={}", serial_number);
    }

    string vid = read_line(sysfs_path + "/idVendor");
    string pid = read_line(sysfs_path + "/idProduct");

    return fmt::format("USB VID:PID={}:{} {}", vid, pid, serial_number);
}


std::tuple<string, string> get_sysfs_info(const string& device_path) {
    string friendly_name;
    string hardware_id;

    const auto device_name = basename(device_path);

    string sys_device_path = fmt::format("/sys/class/tty/{}/device", device_name);
    if (device_name.compare(0, 6, "ttyUSB") == 0) {
        sys_device_path = dirname(dirname(realpath(sys_device_path)));

        if (path_exists(sys_device_path)){
            friendly_name = usb_sysfs_friendly_name(sys_device_path);
            hardware_id = usb_sysfs_hw_string(sys_device_path);
        }
    } else if (device_name.compare(0, 6, "ttyACM") == 0) {
        sys_device_path = dirname(realpath(sys_device_path));

        if (path_exists(sys_device_path)) {
            friendly_name = usb_sysfs_friendly_name(sys_device_path);
            hardware_id = usb_sysfs_hw_string(sys_device_path);
        }
    } else {
        // Try to read ID string of PCI device
        string sys_id_path = sys_device_path + "/id";
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
    std::vector<std::string> devices_found = glob({
                                            "/dev/ttyACM*",
                                            "/dev/ttyS*",
                                            "/dev/ttyUSB*",
                                            "/dev/tty.*",
                                            "/dev/cu.*"
                                        });

    vector<Solace::IO::SerialPortInfo> results;
    results.reserve(devices_found.size());

    for (const auto& device : devices_found) {
        string friendly_name;
        string hardware_id;
        std::tie(friendly_name, hardware_id) = get_sysfs_info(device);

        results.emplace_back(
                    Solace::Path::parse(device),
                    Solace::String(friendly_name),
                    Solace::String(hardware_id));
    }

    return { results };
}
