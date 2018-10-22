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


StringView viewString(std::string const& str) {
    return StringView(str.data(), str.size());
}


auto
readLine(Path const& file) {
    auto const filePathStr = file.toString();
    auto line = std::string{filePathStr.view().data(), filePathStr.view().size()};
    auto ifs = std::ifstream{line, std::ifstream::in};

    if (ifs) {
        getline(ifs, line);

        return line;
    }

    return std::string{};
}


String
usb_sysfs_friendly_name(Path const& sys_usb_path) {
    auto const manufacturer = readLine(makePath(sys_usb_path, StringLiteral{"manufacturer"}));
    auto const product = readLine(makePath(sys_usb_path, StringLiteral{"product"}));
    auto const serial = readLine(makePath(sys_usb_path, StringLiteral{"serial"}));

    if (manufacturer.empty() && product.empty() && serial.empty()) {
        return String{};
    }

    return makeStringJoin(" ", viewString(manufacturer), viewString(product), viewString(serial));
}


String
usb_sysfs_hw_string(Path const& sysfs_path) {
    auto const vid = readLine(makePath(sysfs_path, StringLiteral{"idVendor"}));
    auto const pid = readLine(makePath(sysfs_path, StringLiteral{"idProduct"}));
    auto const serial_number = readLine(makePath(sysfs_path, StringLiteral{"serial"}));

    return makeString(
                StringLiteral{"USB VID:PID="},
                viewString(vid), StringLiteral(":"),  viewString(pid),
                serial_number.empty() ? StringLiteral() : StringLiteral(" SNR="),
                viewString(serial_number));
}


std::tuple<String, String>
get_sysfs_info(IO::PlatformFilesystem const& fs, Path const& devicePath) {
    static const Path SYS_TTY_PATH = makePath(Path::Root,
                                                     StringLiteral{"sys"},
                                                     StringLiteral{"class"},
                                                     StringLiteral{"tty"});

    String friendly_name;
    String hardware_id;

    auto device_name = devicePath.getBasename();
    auto sys_device_path = makePath(SYS_TTY_PATH, device_name, StringLiteral{"device"});

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
        const auto sys_id_path = makePath(sys_device_path, "id");

        if (fs.exists(sys_id_path)) {
            auto const hwIdLine = readLine(sys_id_path);
            hardware_id = makeString(viewString(hwIdLine));
        }
    }

    if (friendly_name.empty()) {
        friendly_name = makeString(device_name);
    }

    if (hardware_id.empty()) {
        hardware_id = makeString(HW_ID_NA);
    }

    return std::make_tuple(std::move(friendly_name), std::move(hardware_id));
}


Array<IO::SerialPortInfo>
Solace::IO::Serial::enumeratePorts() {

    auto fs = PlatformFilesystem();
    auto devices_found = fs.glob({  "/dev/ttyACM*",
                                    "/dev/ttyS*",
                                    "/dev/ttyUSB*",
                                    "/dev/tty.*",
                                    "/dev/cu.*"
                                    });

    // NOTE: The vector is used here even thou the size is know already because we don't want to initialize structs yet
    auto results = makeVector<Solace::IO::SerialPortInfo>(devices_found.size());
    for (auto&& device : devices_found) {
        String friendly_name;
        String hardware_id;
        std::tie(friendly_name, hardware_id) = get_sysfs_info(fs, device);

        results.emplace_back(std::move(device), std::move(friendly_name), std::move(hardware_id));
    }

    return results.toArray();
}
