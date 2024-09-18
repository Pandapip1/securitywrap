# securitywrap

Sometimes, you have an application that's picky about which users / groups it's being run as, and whether they match the real user id or group id. This is a simple program that allows for easy wrappers to be created to ensure that the application is run as the correct user and group.

Capability support coming SOON™

## Usage

```bash
cmake . [ -DSET_UID=<user_or_uid> | -DRESET_UID=ON ] [ -DSET_REAL_UID=<user_or_uid> ] [ -DSET_GID <group_or_gid> | -DRESET_GID=ON ] [ -DSET_REAL_GID <group_or_gid> ] -DWRAP_EXECUTABLE=/path/to/application
make
```

The resulting wrapper should be setuid root, and **is not portable**. It relies on `WRAP_EXECUTABLE` being available at the path specified at compile time, and on the specified group(ID)(s) / user(ID)(s) existing on the system.
