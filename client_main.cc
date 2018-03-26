#include <iostream>
#include <memory>

#include "pow_client.h"

int main(int argc, char const *argv[]) {
  ProofOfWorkGameClient client("localhost:50051");
  client.Start();

  while (client.IsConnected()) {
    if (auto puzzle = client.PollNextPuzzle(1)) {
      // Your code goes here
      // Hint: you have to brute force the hash inputs
      std::cout << "Got new puzzle." << std::endl;
    }
  }
  return 0;
}
