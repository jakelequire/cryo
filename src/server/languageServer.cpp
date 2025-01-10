/********************************************************************************
 *  Copyright 2024 Jacob LeQuire                                                *
 *  SPDX-License-Identifier: Apache-2.0                                         *
 *    Licensed under the Apache License, Version 2.0 (the "License");           *
 *    you may not use this file except in compliance with the License.          *
 *    You may obtain a copy of the License at                                   *
 *                                                                              *
 *    http://www.apache.org/licenses/LICENSE-2.0                                *
 *                                                                              *
 *    Unless required by applicable law or agreed to in writing, software       *
 *    distributed under the License is distributed on an "AS IS" BASIS,         *
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
 *    See the License for the specific language governing permissions and       *
 *    limitations under the License.                                            *
 *                                                                              *
 ********************************************************************************/
#include "server/languageServer.hpp"

namespace Cryo
{
    // This is the public facing function to spin up the language server.
    void CryoLanguageServer::startServer(void)
    {
        _internal_start_server();
    }

    // This function is designed to start up a server in a separate thread.
    // This is so that it won't interfere with the main program compilation.
    void CryoLanguageServer::_internal_start_server(void)
    {
        std::thread serverThread([this]()
                                 {
            int server_fd, new_socket;
            struct sockaddr_in address;
            int opt = 1;
            int addrlen = sizeof(address);

            // Creating socket file descriptor
            if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
            {
                perror("socket failed");
                exit(EXIT_FAILURE);
            }

            // Forcefully attaching socket to the port
            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
            {
                perror("setsockopt");
                exit(EXIT_FAILURE);
            }
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(serverPort);

            // Bind the socket to the network address and port
            if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
            {
                perror("bind failed");
                exit(EXIT_FAILURE);
            }
            if (listen(server_fd, 3) < 0)
            {
                perror("listen");
                exit(EXIT_FAILURE);
            }
            std::cout << "Server started on port " << serverPort << std::endl;

            while (true)
            {
                if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                std::cout << "Connection accepted" << std::endl;
                close(new_socket);
            } });

        // Detach the thread so it runs independently
        serverThread.detach();
    }

} // namespace Cryo
