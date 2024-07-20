#include <sys/wait.h>

#include "common.hxx"

constexpr std::int64_t TEST_PORT = 1234567;

class EventLoopTest : public ::testing::TestWithParam<QueryArray> {
 protected:
  Server server;
  Client client;
  // std::thread serverThread;
  pid_t serverPid;

  /**
   * @brief Set up the text fixture.
   *
   * Initializes the server in a separate process. The reason for this is to
   * give the client the chance to respond. In a real scenario, the client and
   * the server would be different applications, with different runtimes.
   */
  void SetUp() override {
    serverPid = fork();
    if (serverPid == 0) {
      server.run();
      exit(0);
    } else if (serverPid > 0) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    } else {
      std::cerr << "Failed to fork server process" << std::endl;
      exit(1);
    }
  }

  /**
   * @brief Tear down the text fixture.
   *
   * Ensures that the server process is properly terminated and waited for.
   */
  void TearDown() override {
    if (serverPid > 0) {
      kill(serverPid, SIGTERM);
      waitpid(serverPid, nullptr, 0);
    }
  }
};

INSTANTIATE_TEST_SUITE_P(QueryTests, EventLoopTest,
                         ::testing::Values(QueryArray{"foo", "bar", "baz"}));

TEST_P(EventLoopTest, SendAndReceive) {
  auto queryList = GetParam();
  client.run(queryList);
}
