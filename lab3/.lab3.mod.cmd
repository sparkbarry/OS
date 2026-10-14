savedcmd_lab3.mod := printf '%s\n'   lab3.o | awk '!x[$$0]++ { print("./"$$0) }' > lab3.mod
