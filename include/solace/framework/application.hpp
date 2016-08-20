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
 * libSolace: Base class for applicatoins
 *	@file		solace/framework/application.hpp
 *	@author		$LastChangedBy: $
 *	@date		$LastChangedDate: $
 *	@brief		Main Application class
 *	ID:			$Id: $
 ******************************************************************************/
#pragma once
#ifndef SOLACE_FRAMEWORK_APPLICATION_HPP
#define SOLACE_FRAMEWORK_APPLICATION_HPP

#include "solace/result.hpp"
#include "solace/error.hpp"
#include "solace/version.hpp"


namespace Solace { namespace Framework {

/**
 * Base class for Applications
 * This class provides a easy to use application flow
 */
class Application {
public:

    /**
     * Get Application version
     * @return Build time version of this application
     */
    virtual Version getVersion() const = nullptr;

    /**
     * Initialise application instance.
     * This call prepares instance for run given command line options.
     *
     * @param argc Number of command line options
     * @param argv Vector of command line arguments
     * @return Initialization result which can be either runnable action that resulted from given command line or
     * an error if application failed to initialized.
     */
    virtual Result<std::function<int()>, Error> init(int argc, char *argv[]) = nullptr;

};

}  // End of namespace Framework
}  // End of namespace Solace
#endif  // SOLACE_FRAMEWORK_APPLICATION_HPP
