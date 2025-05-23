#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>    // For inet_ntop() and sockaddr_in/sockaddr_in6
#include <iphlpapi.h>


// Link with Iphlpapi.lib
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

int main(int argc, char* argv[]) {
    printf("Hello from ripconfig!\n\n");

    ULONG            ulOutBufLen;
    DWORD            dwRetVal;

    printf("Starting GetAdaptersAddresses enumeration...\n");
    /* Declare and initialize variables */

    unsigned int i = 0;

    // Set the flags to pass to GetAdaptersAddresses
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    // default to unspecified address family (both)
    ULONG family = AF_UNSPEC;

    LPVOID lpMsgBuf = NULL;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG Iterations = 0;

    PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
    PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
    IP_ADAPTER_DNS_SERVER_ADDRESS* pDnServer = NULL;
    IP_ADAPTER_PREFIX* pPrefix = NULL;
    PIP_ADAPTER_GATEWAY_ADDRESS pGateway = NULL;

    if (argc != 2) {
        printf(" usage: getadapteraddresses family\n");
        printf("        getadapteraddresses 4 (for ipv4)\n");
        printf("        getadapteraddresses 6 (for ipv6)\n");
        printf("        getadapteraddresses a (for both ipv4 and ipv6)\n");
        exit(1);
    }

    if (atoi(argv[1]) == 4)
        family = AF_INET;
    else if (atoi(argv[1]) == 6)
        family = AF_INET6;
  /*  else if (atoi(argv[1]) == "")
        family = AF_UNSPEC;*/

    printf("Calling GetAdaptersAddresses function with family = ");
    if (family == AF_INET)
        printf("AF_INET\n");
    if (family == AF_INET6)
        printf("AF_INET6\n");
    if (family == AF_UNSPEC)
        printf("AF_UNSPEC\n\n");

    // Allocate a 15 KB buffer to start with.
    outBufLen = WORKING_BUFFER_SIZE;

    do {

        pAddresses = (IP_ADAPTER_ADDRESSES*)MALLOC(outBufLen);
        if (pAddresses == NULL) {
            printf
            ("Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n");
            exit(1);
        }

        dwRetVal =
            GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            FREE(pAddresses);
            pAddresses = NULL;
        }
        else {
            break;
        }

        Iterations++;

    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

    if (dwRetVal == NO_ERROR) {
        // If successful, output some information from the data we received
        pCurrAddresses = pAddresses;
        
        while (pCurrAddresses) {
            /*printf("\n\tLength of the IP_ADAPTER_ADDRESS struct: %ld\n",
                pCurrAddresses->Length);
            printf("\tIfIndex (IPv4 interface): %u\n", pCurrAddresses->IfIndex);
            printf("\tAdapter name: %s\n", pCurrAddresses->AdapterName);*/
            printf("\nFriendly name: %wS\n", pCurrAddresses->FriendlyName); //useful
            printf("\tConnection-specific DNS Suffix: %wS\n", pCurrAddresses->DnsSuffix);  //useful

            // ZT CODE
            pUnicast = pCurrAddresses->FirstUnicastAddress;
            if (pUnicast != NULL) {
                for (i = 0; pUnicast != NULL; i++) {
                    char ip_buffer[INET6_ADDRSTRLEN];
                    void* lpSockaddr;
                    const char* strIpAddr;
                    ADDRESS_FAMILY sa_family;
                    sa_family = pUnicast->Address.lpSockaddr->sa_family;
                    if (sa_family == AF_INET) {
                        lpSockaddr = &((SOCKADDR_IN*)pUnicast->Address.lpSockaddr)->sin_addr;
                        strIpAddr = inet_ntop(sa_family, lpSockaddr, ip_buffer, sizeof(ip_buffer));
                        if (strIpAddr != NULL) {
                            printf("\t\tIPv4 Address: %s\n", strIpAddr);
                        }
                        else {
                            printf("\tAddress conversion failed\n");
                        }
                    }
                    else if (sa_family == AF_INET6) {

                        lpSockaddr = &((SOCKADDR_IN6*)pUnicast->Address.lpSockaddr)->sin6_addr;
                        strIpAddr = inet_ntop(sa_family, lpSockaddr, ip_buffer, sizeof(ip_buffer));
                        if (strIpAddr != NULL) {
                            printf("\t\tLink-local IPv6 Address: %s\n", strIpAddr);
                        }
                        else {
                            printf("\tAddress conversion failed\n");
                        }
                    }
                    else {
                        break;
                    }

                    pUnicast = pUnicast->Next;
                }
                //printf("\tNumber of Unicast Addresses: %d\n", i);
                
            }
            else
                printf("\tNo Unicast Addresses\n");

            // ZT - Subnet Mask - 
            pGateway = pCurrAddresses->FirstGatewayAddress;
            if (pGateway != NULL) {
                // Add code here
                continue;
            }
            else {
                // No gateways :(
                printf("\tNo gateways identified.");
            }

            // ZT - Default Gateway - pCurrAddresses -> FirstPrefix;

            pAnycast = pCurrAddresses->FirstAnycastAddress;
            if (pAnycast) {
                for (i = 0; pAnycast != NULL; i++)
                    pAnycast = pAnycast->Next;
                printf("\tNumber of Anycast Addresses: %d\n", i);
            }
            else
                printf("\tNo Anycast Addresses\n");

            pMulticast = pCurrAddresses->FirstMulticastAddress;
            if (pMulticast) {
                for (i = 0; pMulticast != NULL; i++)
                    pMulticast = pMulticast->Next;
                printf("\tNumber of Multicast Addresses: %d\n", i);
            }
            else
                printf("\tNo Multicast Addresses\n");

            pDnServer = pCurrAddresses->FirstDnsServerAddress;
            if (pDnServer) {
                for (i = 0; pDnServer != NULL; i++)
                    pDnServer = pDnServer->Next;
                printf("\tNumber of DNS Server Addresses: %d\n", i);
            }
            else
                printf("\tNo DNS Server Addresses\n");

            
            //printf("\tDescription: %wS\n", pCurrAddresses->Description);
            

            /*if (pCurrAddresses->PhysicalAddressLength != 0) {
                printf("\tPhysical address: ");
                for (i = 0; i < (int)pCurrAddresses->PhysicalAddressLength;
                    i++) {
                    if (i == (pCurrAddresses->PhysicalAddressLength - 1))
                        printf("%.2X\n",
                            (int)pCurrAddresses->PhysicalAddress[i]);
                    else
                        printf("%.2X-",
                            (int)pCurrAddresses->PhysicalAddress[i]);
                }
            }*/
            /*printf("\tFlags: %ld\n", pCurrAddresses->Flags);
            printf("\tMtu: %lu\n", pCurrAddresses->Mtu);
            printf("\tIfType: %ld\n", pCurrAddresses->IfType);
            printf("\tOperStatus: %ld\n", pCurrAddresses->OperStatus);
            printf("\tIpv6IfIndex (IPv6 interface): %u\n",
                pCurrAddresses->Ipv6IfIndex);
            printf("\tZoneIndices (hex): ");
            for (i = 0; i < 16; i++)
                printf("%lx ", pCurrAddresses->ZoneIndices[i]);
            printf("\n");

            printf("\tTransmit link speed: %I64u\n", pCurrAddresses->TransmitLinkSpeed);
            printf("\tReceive link speed: %I64u\n", pCurrAddresses->ReceiveLinkSpeed);

            pPrefix = pCurrAddresses->FirstPrefix;
            if (pPrefix) {
                for (i = 0; pPrefix != NULL; i++)
                    pPrefix = pPrefix->Next;
                printf("\tNumber of IP Adapter Prefix entries: %d\n", i);
            }
            else
                printf("\tNumber of IP Adapter Prefix entries: 0\n");

            printf("\n");*/

            pCurrAddresses = pCurrAddresses->Next;
        }
    }
    else {
        printf("Call to GetAdaptersAddresses failed with error: %d\n",
            dwRetVal);
        if (dwRetVal == ERROR_NO_DATA)
            printf("\tNo addresses were found for the requested parameters\n");
        else {

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                // Default language
                (LPTSTR)&lpMsgBuf, 0, NULL)) {
                printf("\tError: %s", lpMsgBuf);
                LocalFree(lpMsgBuf);
                if (pAddresses)
                    FREE(pAddresses);
                exit(1);
            }
        }
    }

    if (pAddresses) {
        FREE(pAddresses);
    }

    
    // Exit
    return 0;
}