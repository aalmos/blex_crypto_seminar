#include <iostream>
#include <memory>

#include "pow_client.h"

int main(int argc, char const *argv[]) {
  if (argc < 2) {
    std::cout << "Please provide player name." << std::endl;
  }

  ProofOfWorkGameClient client("localhost:50051");
  client.Start();

  while (client.IsConnected()) {
    if (auto puzzle = client.PollNextPuzzle(1)) {
      std::cout << "Got new puzzle." << std::endl;
    }
  }
  return 0;
}