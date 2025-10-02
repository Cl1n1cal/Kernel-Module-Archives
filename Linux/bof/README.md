### Description
This kernel module is using the (vulnerable) raw_copy_to_user and raw_copy_from_user.
Since the attacker in this instance can choose how many bytes they want to read or write from the kernel module they are able
to get both a canary and kernel leak and a BOF to take control of RIP. The kernel module buffers were coded with 100 bytes in size to purposely disalign them from 8 byte offsets to make it a little more inconvenient in GDB for practice reasons. The module also features two different buffers
for reading and writing since this means that the stack leak is not 100% reliable and thus leaking a stack addr will not make the attacker able to
reliably calculate the address of the buffer that they are writing to.<br>

Best regards,
Cl1nical