#include <stdio.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    printf("Hello from ripconfig!\n");

    // Print out any command-line arguments
    printf("You provided %d arguments:\n", argc);

    for (int i = 0; i < argc; i++) {
        printf("Argument %d: %s\n", i, argv[i]);
    }

    // Declare variables passed as parameters to GetAdaptersInfo and error checking
    IP_ADAPTER_INFO* pAdapterInfo;
    ULONG            ulOutBufLen;
    DWORD            dwRetVal;

    // Allocate Memory
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    // Call to GetAdaptersInfo to get Size Needed - Designed to Fail
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
    }
    // Return GetAdaptersInfo
    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != ERROR_SUCCESS) {
        printf("GetAdaptersInfo call failed with %d\n", dwRetVal);
    }

    // Hungarian for: pointer to IP_ADAPTER_INFO
    PIP_ADAPTER_INFO pAdapter = pAdapterInfo;

    // While statement, which reads each
    while (pAdapter) {
        // Not sure what adapter name is used for
        //printf("Adapter Name: %s\n", pAdapter->AdapterName);
        // output comes from ipconfig /all - why is the syntax `pAdapter->Description`? Presumably it is accessing the `Description` member of the `pAdapter`?
        printf("Adapter Desc: %s\n", pAdapter->Description);
        // Get MAC Address
        printf("\tAdapter Addr: \t");
        for (UINT i = 0; i < pAdapter->AddressLength; i++) {
            if (i == (pAdapter->AddressLength - 1))
                // if the last byte, print newline after printing byte
                printf("%.2X\n", (int)pAdapter->Address[i]);
            else
                // otherwise print : after byte
                printf("%.2X:", (int)pAdapter->Address[i]);
        }
        // why does thi string have %s in it?
        printf("IP Address: %s\n", pAdapter->IpAddressList.IpAddress.String);
        printf("Subnet Mask: %s\n", pAdapter->IpAddressList.IpMask.String);
        printf("\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
        printf("\t***\n");
        if (pAdapter->DhcpEnabled) {
            printf("\tDHCP Enabled: Yes\n");
            printf("\t\tDHCP Server: \t%s\n", pAdapter->DhcpServer.IpAddress.String);
        }
        else
            printf("\tDHCP Enabled: No\n");

        // Next member, if last of linked list, would be NULL (evalutating to false, and ending loop)
        pAdapter = pAdapter->Next;
    }

    // Free memory allocated
    if (pAdapterInfo)
        free(pAdapterInfo);

    // Exit
    return 0;
}