# securitywrap

Sometimes, you have an application that's picky about which users / groups it's being run as, and whether they match the real user id or group id. This is a simple interpreter that allows for easy wrappers to be created to ensure that the application is run as the correct user and group.

Capability support coming SOON™

## Usage

```bash
#!/bin/env bash
exec /path/to/securitywrap [--set-uid <user_or_uid> | --reset-uid ] [--set-real-uid <user_or_uid>] [--set-gid <group_or_gid> | --reset-gid] [--set-real-gid <group_or_gid>] /path/to/application $@
```

The wrapper should be setuid root. The securitywrap binary should be read-only and owned by root, and MUST NOT have the setuid bit set.
