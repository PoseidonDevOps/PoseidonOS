package develcmds_test

import (
	"cli/cmd"
	"cli/cmd/globals"
	"cli/cmd/testmgr"
	"io/ioutil"
	"os"
	"testing"
)

// This testing tests if the request is created well in JSON form from the command line.
func TestResetMBRCommandReq(t *testing.T) {

	// Command creation
	rootCmd := cmd.RootCmd

	// mj: For testing, I temporarily redirect log output to buffer.
	rescueStdout := os.Stdout
	r, w, _ := os.Pipe()
	os.Stdout = w

	globals.IsTestingReqBld = true
	// Execute the command to test with argument
	testmgr.ExecuteCommand(rootCmd, "devel", "reset-event-wrr", "--json-req")

	w.Close()
	out, _ := ioutil.ReadAll(r)
	os.Stdout = rescueStdout

	// TODO(mj): Currently, we compare strings to test the result.
	// This needs to change. i) Parsing the JSON request and compare each variable with desired values.
	expected := `{"command":"RESETMBR","rid":"fromfakeclient"s}`

	if expected != string(out) {
		t.Errorf("Expected: %q Output: %s", expected, string(out))
	}
}
