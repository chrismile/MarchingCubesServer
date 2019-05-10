/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2019, Christoph Neuhauser
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <glm/glm.hpp>
#include "BinaryStream.hpp"
#include "mc/MarchingCubes.hpp"
#include "mc/Defines.hpp"

/**
 * As the data transfer to the application can be quite large, the maximum message size is set to 320MB.
 */
struct asio_large_msg : public websocketpp::config::asio {
    static const size_t max_message_size = 320000000; // 320MB
};

typedef websocketpp::server<asio_large_msg> server;
typedef server::message_ptr message_ptr;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

static MarchingCubesImpl *mcImpl = NULL;

/**
 * This function is called when the server receives a request.
 * The request consists of a Cartesian grid storing a discrete scalar field.
 * As an answer, the server creates and sends a triangular approximation of the iso surface (at iso level 0) as a list
 * of triangle points.
 * @param s The server.
 * @param hdl The connection handle.
 * @param msg The received message.
 */
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
    if (msg->get_opcode() != websocketpp::frame::opcode::binary) {
        std::cerr << "Expected binary opcode." << std::endl;
        return;
    }
    std::cout << "Received request." << std::endl;

    BinaryReadStream readStream((const void *)msg->get_payload().data(), msg->get_payload().size());

    float isoLevel = 0.0f;

    // Read number of cells in x, y and z direction (for now uniform).
    uint32_t nx = 0;
    readStream.read(nx);
    std::cout << "nx: " << nx << std::endl;

    // Allocate memory for cartesian grid and read the data.
    std::vector<CartesianGridCorner> cartesianGrid;
    cartesianGrid.resize(nx*nx*nx);
    readStream.read((void*)&cartesianGrid.front(), sizeof(CartesianGridCorner)*nx*nx*nx);

    // Launch the marching cubes algorithm for creating the iso surface and measure the time it took.
    auto startLoad = std::chrono::system_clock::now();
    std::vector<glm::vec3> trianglePoints = mcImpl->marchingCubes(nx, isoLevel, cartesianGrid);
    auto endLoad = std::chrono::system_clock::now();
    auto elapsedLoad = std::chrono::duration_cast<std::chrono::milliseconds>(endLoad - startLoad);
    std::cout << "Marching cubes finished in: " << std::to_string(elapsedLoad.count()/1000.0f) << "s" << std::endl;
    std::cout << "#triangle points: " << trianglePoints.size() << std::endl;

    // Finally, send the triangle vertex list to the client.
    try {
        s->send(hdl, (void *)&trianglePoints.front(), sizeof(glm::vec3) * trianglePoints.size(),
                websocketpp::frame::opcode::binary);
    } catch (websocketpp::exception const & e) {
        std::cerr << "Send failed: " << "(" << e.what() << ")" << std::endl;
    }
}

/**
 * Waits for the user to type a command in the command line that closes the server.
 */
void listenForClose() {
    bool running = true;
    do {
        std::string command;
        std::getline(std::cin, command);
        if (command == "quit") {
            running = false;
        } else {
            std::cerr << "Unknown command!" << std::endl;
        }
    } while (running);
}

/**
 * A wrapper for creating a thread that runs the WebSocket server service loop.
 * @param mcServer The server object.
 */
void runServer(server *mcServer) {
    mcServer->run();
}

int main(int argc, char *argv[]) {
    // Create a server endpoint
    server mcServer;
    std::cout << "Please type 'quit' for closing the server..." << std::endl;

    mcImpl = new MarchingCubesImpl;
    mcImpl->init();

    try {
        // Set logging settings
        mcServer.set_access_channels(websocketpp::log::alevel::all);
        mcServer.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize Boost::Asio
        mcServer.init_asio();

        // Register the message handler
        mcServer.set_message_handler(bind(&on_message, &mcServer, ::_1, ::_2));

        // Listen on port 17279
        mcServer.listen(17279);

        // Start the server accept loop
        mcServer.start_accept();

        // Start the ASIO io_service run loop
        std::thread serverThread = std::thread(runServer, &mcServer);

        // Wait for the user to type a command in the command line that closes the server.
        listenForClose();
        mcServer.stop();
        serverThread.join();
    } catch (websocketpp::exception const &e) {
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cerr << "An unknown exception occured." << std::endl;
    }

    mcImpl->quit();
    delete mcImpl;

    return 0;
}
