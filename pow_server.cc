#include <algorithm>
#include <random>
#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>

#include "crypto_helpers.h"
#include "proof_of_work_game.grpc.pb.h"

using google::protobuf::Empty;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;

namespace {
const size_t kPuzzleBaseLength = 16;
const uint32_t kPuzzleDifficulty = 5;

Puzzle GetNextPuzzle(size_t base_length, uint32_t difficulty) {
  Puzzle result;
  result.set_base(crypto_helpers::GenerateRandomBytes(base_length));
  result.set_hash_prefix(crypto_helpers::BytesToHexString(
      crypto_helpers::GenerateRandomBytes(difficulty))
                             .substr(0, difficulty));
  return result;
}

bool CheckPuzzleSolution(const Puzzle &puzzle,
                         const Solution &solution) {
  if (solution.solution().compare(0, puzzle.base().size(),
                                  puzzle.base()) != 0) {
    return false;
  }

  return crypto_helpers::Sha256HexString(solution.solution())
             .compare(0, puzzle.hash_prefix().size(),
                      puzzle.hash_prefix()) == 0;
}
} // namespace helpers

class ProofOfWorkGameServerImpl : public ProofOfWorkGame::Service {
 private:
  std::mutex mutex_;
  Puzzle current_puzzle_;
  uint32_t difficulty_;
  std::vector<ServerReaderWriter<Puzzle, Solution> *> streams_;
  std::unordered_map<std::string, uint32_t> scores_;

  bool ProposeSolution(const Solution &solution) {
    if (solution.player_name().empty() || solution.solution().empty()) {
      return false;
    }

    if (!CheckPuzzleSolution(current_puzzle_, solution)) {
      return false;
    }

    std::cout << "Player '" << solution.player_name()
              << "' solved the puzzle." << std::endl;

    RecordWinner(solution.player_name());
    current_puzzle_ = GetNextPuzzle(kPuzzleBaseLength, difficulty_);

    return true;
  }

  void RecordWinner(const std::string &player_name) {
    if (scores_.find(player_name) != scores_.end()) {
      scores_[player_name]++;
    } else {
      scores_[player_name] = 0;
    }
  }

  void ProcessSolution(const Solution &solution) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!ProposeSolution(solution)) {
      return;
    }
    std::shuffle(streams_.begin(), streams_.end(),
                 std::mt19937(std::random_device()()));
    for (auto *stream : streams_) {
      stream->Write(current_puzzle_);
    }
  }

  void AddStreamAndSendFirstPuzzle(
      ServerReaderWriter<Puzzle, Solution> *stream) {
    std::unique_lock<std::mutex> lock(mutex_);
    streams_.push_back(stream);
    stream->Write(current_puzzle_);
  }

  void RemoveStream(ServerReaderWriter<Puzzle, Solution> *stream) {
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto it = streams_.begin(); it != streams_.end(); it++) {
      if (*it == stream) {
        streams_.erase(it);
        break;
      }
    }
  }

 public:
  explicit ProofOfWorkGameServerImpl(uint32_t difficulty)
      : difficulty_(difficulty) {
    current_puzzle_ = GetNextPuzzle(kPuzzleBaseLength, difficulty);
  }

  Status SolvePuzzles(ServerContext *context,
                      ServerReaderWriter<Puzzle, Solution> *stream) override {
    std::cout << "Client '" << context->peer()
              << "' connected." << std::endl;

    AddStreamAndSendFirstPuzzle(stream);
    Solution solution;

    while (stream->Read(&solution)) {
      ProcessSolution(solution);
    }

    std::cout << "Client '" << context->peer()
              << "' left." << std::endl;

    RemoveStream(stream);
    return Status::OK;
  }

  Status GetScoreBoard(ServerContext* context, const Empty* request,
                       ScoreBoard* response) override {
    for (const auto& score : scores_) {
      Record record;
      record.set_player_name(score.first);
      record.set_score(score.second);
      *response->add_records() = record;
    }
    return Status::OK;
  }
};

int main(int argc, char const *argv[]) {
  std::string server_address("0.0.0.0:50051");
  ProofOfWorkGameServerImpl service(kPuzzleDifficulty);
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << "." << std::endl;
  server->Wait();
}