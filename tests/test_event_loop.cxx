#include <sys/wait.h>

#include "common.hxx"

constexpr std::int64_t TEST_PORT = 1234567;

/**
 * @class EventLoopTest
 * @brief Test fixture for testing the event loop functionality
 *
 * This class sets up a server and client environment for testing the main event
 * loop of the app. The server is ran in a separate process to simulate a real
 * run scenario as if the client and server were different remote application
 * with different runtimes
 */
class EventLoopTest : public ::testing::TestWithParam<CommandList> {
protected:
  Server server;
  Client client;
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
      server.run(TEST_PORT);
      exit(0);
    } else if (serverPid > 0) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    } else {
      std::cerr << "Failed to fork server process" << '\n';
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

/**
 * @brief Instantiates the parametrized test suite for EventLoopTest.
 *
 * This macro provides the test suite for the even loop test, a CommandList for
 * the client to run.
 */
INSTANTIATE_TEST_SUITE_P(QueryTests, EventLoopTest,
                         ::testing::Values(CommandList{"get", "k"}));

/**
 * @brief Test case for sending a list of queries to the server
 *
 * Runs the client on the dummy query array.
 */
TEST_P(EventLoopTest, SendAndReceive) {
  auto const &queryList = GetParam();
  client.run(queryList, TEST_PORT);
}
