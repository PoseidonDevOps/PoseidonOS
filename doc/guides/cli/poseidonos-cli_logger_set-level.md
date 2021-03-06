## poseidonos-cli logger set-level

Set the filtering level of logger.

### Synopsis


Set the filtering level of logger.

Syntax:
	poseidonos-cli logger set-level --level [debug | info | warn | err | critical]
          

```
poseidonos-cli logger set-level [flags]
```

### Options

```
  -h, --help           help for set-level
      --level string   The level of logger to set.
                       	
                       - critical: events that make the system not available.
                       - err: events when PoseidonOS cannot process the request
                       	because of an internal problem. 
                       - warn: events when unexpected user input has been detected. 
                       - debug: logs when the system is working properly.
                       - info: logs for debug binary.
```

### Options inherited from parent commands

```
      --debug         Print response for debug.
      --fs string     Field separator for the output. (default "|")
      --ip string     Set IPv4 address to PoseidonOS for this command. (default "127.0.0.1")
      --json-req      Print request in JSON form.
      --json-res      Print response in JSON form.
      --port string   Set the port number to PoseidonOS for this command. (default "18716")
      --unit          Display unit (B, KB, MB, ...) when displaying capacity.
```

### SEE ALSO

* [poseidonos-cli logger](poseidonos-cli_logger.md)	 - Logger commands for PoseidonOS.

