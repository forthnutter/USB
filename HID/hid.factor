! Copyright (C) 2011 Joseph Louis Moschini.
! See http://factorcode.org/license.txt for BSD license.

USING: kernel alien alien.c-types alien.syntax alien.data alien.strings
       namespaces locals literals math math.parser prettyprint
       windows windows.kernel32 windows.ddk.setupapi windows.ddk.hid
       windows.ole32 windows.errors arrays classes.struct memory accessors
       sequences byte-arrays windows.types tools.continuations ;

IN: USB.HID

TUPLE: hds devdetail outputreport outputreportsz inputreport inputreportsz
    detaildatasz featurereport featurereportsz devhandle devoverlapped
    devreadpending count ;

TUPLE: hid-device-info index path vendor-id product-id
    usage-page usage in-rep-len out-rep-len feature-rep-len ;


CONSTANT: DEV_NUM 10
SYMBOLS: DevSelected DevDetailSelected ;
SYMBOLS: DevHandle  ;
SYMBOLS: DevDetailDataSz  DevFeatureReportSz ;
SYMBOLS: hdevice devcount ;


! ****************************************
! Create device select
! ****************************************
: <hds> ( -- hds )
    hds new
    f >>devdetail
    f >>devhandle
    f >>outputreport
    0 >>outputreportsz
    f >>inputreport
    0 >>inputreportsz
    0 >>detaildatasz
    f >>featurereport
    0 >>featurereportsz
    f >>devreadpending
    f >>devoverlapped
    f >>count
;

! ****************************************
! Create device select
! ****************************************
: <hid-device-info> ( -- hid-device-info )
    hid-device-info new
    0 >>index
    f >>path
    0 >>vendor-id
    0 >>product-id
;

! ****************************************
! Description:  Enumerate HID devices
! ****************************************
:: hid-enumerate ( vid pid -- vector )
    
    ! Get GUID for all System HIDs
    ! HidD_GetHidGuid(&HidGuid)
    GUID <struct> :> hidguid
    hidguid HidD_GetHidGuid
    
    ! Get Device Information for all present devices
    hidguid f f flags{ DIGCF_PRESENT DIGCF_DEVICEINTERFACE }
    SetupDiGetClassDevs :> devinfo
  
    ! generate memory for SP_DEVICE_INTERFACE_DATA structure
    SP_DEVICE_INTERFACE_DATA <struct> :> devdata
    devdata [ byte-length ] keep cbSize<< ! save size of structure

    0 :> index!
    f :> exit!
    f :> devdetail!
    0 DWORD <ref> :> length
    f :> sz!
    V{ } clone :> detailvector
    0 :> count!
    ! Scan all Devices settup the while loop
    [ exit not ]
    [
        devinfo f hidguid index devdata
        SetupDiEnumDeviceInterfaces 0 =
      
        ! if (exit) break;
        [ t exit! ]
        [
            ! Get Device Interface Details - Get Length
            devinfo devdata f 0 length f
            SetupDiGetDeviceInterfaceDetail drop
            !  /* Allocate memory for Device Detailed Data */
            length DWORD deref <byte-array>
            SP_DEVICE_INTERFACE_DETAIL_DATA_W memory>struct
            devdetail!

            !  Sz = Length;
            length DWORD deref sz!

            !  /* Set cbSize in the DevDetail structure */
            !  DevDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            devdetail [ byte-length ] keep cbSize<<

            !  /* Get Device Interface Details */
            devinfo devdata devdetail sz length f
            SetupDiGetDeviceInterfaceDetail
            1 =
            [
                !  Create File for Device Read/Write
                devdetail DevicePath>>
                0 flags{ FILE_SHARE_READ FILE_SHARE_WRITE }
                f OPEN_EXISTING 0 f
                CreateFile :> devhandle

                ! if (DevHandle == INVALID_HANDLE_VALUE) continue;
                devhandle INVALID_HANDLE_VALUE = not
                [
                    ! Get the vender id  and product id
                    HIDD_ATTRIBUTES <struct> :> attrib
                    devhandle attrib HidD_GetAttributes drop
                    
                    ! do we have a match or all
                    vid 0 = pid 0 = and
                    attrib VendorID>> vid = attrib ProductID>> pid = and
                    or
                    [
                        ! create a device structure 
                        <hid-device-info> :> device
                        device

                        ! index of the device
                        count >>index
                        
                        ! get the path turn it into a readble string
                        devdetail DevicePath>> >>path

                        ! save the vendor id
                        attrib VendorID>> >>vendor-id
                        ! save the product id
                        attrib ProductID>> >>product-id
                        
                        ! Get Preparsed Data
                        devhandle
                        8 <byte-array> :> PreparsedData
                        PreparsedData
                        HidD_GetPreparsedData 1 =
                        [
                            !  Get Device's Capabilities
                            PreparsedData DWORD deref <alien>
                            HIDP_CAPS <struct> :> capabilities
                            capabilities
                            HidP_GetCaps
                            HIDP_STATUS_SUCCESS =
                            [
                                ! Free the PreparsedData
                                PreparsedData DWORD deref <alien>
                                HidD_FreePreparsedData drop

                                ! save usage page
                                capabilities UsagePage>> >>usage-page
                                ! save usage
                                capabilities Usage>> >>usage

                                ! InputReportByteLength;
                                capabilities InputReportByteLength>>
                                >>in-rep-len
                                
                                ! OutputReportByteLength;
                                capabilities OutputReportByteLength>>
                                >>out-rep-len
                              
                                ! FeatureReportByteLength;
                                capabilities FeatureReportByteLength>>
                                >>feature-rep-len
                                
                            ] when ! hid caps
                        ] when ! hid preparsed
                        


                        detailvector push  ! save every thing in vector
                        ! Count++
                        count 1 + count!
                        !   DevDetail = NULL
                        f devdetail!
                    ] when
                    ! close off the handle here
                    devhandle CloseHandle drop
                ] when
            ] when
        ] if
        index 1 + index!
    ] while
 
   ! SetupDiDestroyDeviceInfoList (DevInfo);
   devinfo SetupDiDestroyDeviceInfoList drop
   
   ! f hid devinfo<<
   ! devcount get
   detailvector
;



! ****************************************
! Description:  Find HID devices
! Return:       int
! ****************************************
:: hid-find-devices ( -- vector )
    
    ! Get GUID for all System HIDs
    ! HidD_GetHidGuid(&HidGuid)
    GUID <struct> :> hidguid
    hidguid HidD_GetHidGuid
    
    ! Get Device Information for all present devices
    ! DevInfo = SetupDiGetClassDevs(&HidGuid, 
    !                             NULL, 
    !                             NULL, 
    !                             (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
    hidguid
    f f
    flags{ DIGCF_PRESENT DIGCF_DEVICEINTERFACE }
    SetupDiGetClassDevs :> devinfo
  
    ! SP_DEVICE_INTERFACE_DATA         DevData;
    SP_DEVICE_INTERFACE_DATA <struct> :> devdata
    ! DevData.cbSize = sizeof(DevData);
    devdata [ byte-length ] keep cbSize<< 

    ! Index = -1;
    0 :> index!
    !  /* Scan all Devices */
    ! DevCount = 0;
    0 devcount set
    ! hid devdetaildata>>
    f :> exit!
    f :> devdetail!
    0 DWORD <ref> :> length
    f :> sz!
    f :> devhandle!
    V{ } clone :> detailvector
    
    [ exit not ]
    [
        ! do {
        !  Index++;
        !  /* Device Interface Element of a Device Information set */
        !  ok = SetupDiEnumDeviceInterfaces(DevInfo, 
        !        0, 
        !        &HidGuid, 
        !        Index, 
        !        &DevData
        !       );
        devinfo f hidguid index devdata
        SetupDiEnumDeviceInterfaces 0 =
      
        !  if (!ok) break;

        [ t exit! ]
        [
            !  /* Get Device Interface Details - Get Length */
            !  ok = SetupDiGetDeviceInterfaceDetail(DevInfo, 
            !        &DevData,
            !        NULL, 
            !        0, 
            !        &Length, 
            !        NULL
            !       );
            devinfo devdata f 0 length f
            SetupDiGetDeviceInterfaceDetail drop

            !  /* Allocate memory for Device Detailed Data */
            !  if (DevDetail) free(DevDetail);
            ! DevDetail get [ f DevDetail set ] [ ] if

            !  DevDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(Length);
            length DWORD deref <byte-array>
            SP_DEVICE_INTERFACE_DETAIL_DATA_W memory>struct
            devdetail!

            !  Sz = Length;
            length DWORD deref sz!

            !  /* Set cbSize in the DevDetail structure */
            !  DevDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            devdetail [ byte-length ] keep cbSize<<

            !  /* Get Device Interface Details */
            !  ok = SetupDiGetDeviceInterfaceDetail(DevInfo,
            !        &DevData, 
            !        DevDetail, 
            !        Length, 
            !        NULL, 
            !        NULL
            !       );
            devinfo devdata devdetail sz length f
            SetupDiGetDeviceInterfaceDetail
            1 =
            [
                !  if (!ok) continue;
                !  /* Create File for Device Read/Write */
                !  DevHandle = CreateFile(DevDetail->DevicePath, 
                !               0, 
                !               FILE_SHARE_READ | FILE_SHARE_WRITE, 
                !               (LPSECURITY_ATTRIBUTES)NULL,
                !               OPEN_EXISTING, 
                !               0, 
                !               NULL
                !              );
                devdetail DevicePath>>
                0 flags{ FILE_SHARE_READ FILE_SHARE_WRITE }
                f OPEN_EXISTING 0 f
                CreateFile devhandle!

                ! if (DevHandle == INVALID_HANDLE_VALUE) continue;
                devhandle INVALID_HANDLE_VALUE = not
                [
                    ! Get Preparsed Data
                    !  ok = HidD_GetPreparsedData(DevHandle, &PreparsedData);
                    devhandle
                    8 <byte-array> :> PreparsedData
                    PreparsedData
                    HidD_GetPreparsedData 1 =
                    [
                        !  Get Device's Capabilities */
                        !  HidP_GetCaps(PreparsedData, &Capabilities);
                        PreparsedData DWORD deref <alien>
                        HIDP_CAPS <struct> :> capabilities
                        capabilities
                        HidP_GetCaps
                        HIDP_STATUS_SUCCESS =
                        [
                            !  /* Free the PreparsedData */
                            !  HidD_FreePreparsedData(PreparsedData);
                            PreparsedData DWORD deref <alien>
                            HidD_FreePreparsedData drop

                            !  /* Remember Device Interface Detail Data
                            !  for acceptable Devices */
                            !  if ((Capabilities.UsagePage == 0xFF00) &&
                            !      (Capabilities.Usage == 0x0001))
                            !  {
                            capabilities UsagePage>> "FF00" hex> =
                            capabilities Usage>>     "0001" hex> =
                            and
                            [
                                !   DevDetailData[DevCount] = DevDetail;
                                
                                <hds> :> device
                                device
                                devcount get >>count
                                devdetail >>devdetail
                                detailvector push  ! save every thing in vector
                           
                                !   DevDetailDataSz[DevCount] = Sz;
                                sz
                                device detaildatasz<<

                                !   DevInputReportSz[DevCount] =
                                !   Capabilities.InputReportByteLength;
                                capabilities InputReportByteLength>>
                                device inputreportsz<<

                                !   DevOutputReportSz[DevCount] =
                                !   Capabilities.OutputReportByteLength;
                                capabilities OutputReportByteLength>>
                                device outputreportsz<<
                              
                                !   DevFeatureReportSz[DevCount] =
                                !   Capabilities.FeatureReportByteLength;
                                capabilities FeatureReportByteLength>>
                                device featurereportsz<<

                                !   if (DevDetailSelected)
                                !     if (!strcmp(DevDetailSelected->DevicePath,
                                !         DevDetailData[DevCount]->DevicePath)) 
                                !            DevSelected = DevCount;
                             
                                !    DevCount++;
                                devcount inc
                                !   DevDetail = NULL;
                                f devdetail!                     
                            ] when
                        ] when
                  ] when
              ] when
          ] when
      ] if
      index 1 + index!

        !  CloseHandle (DevHandle);
        devhandle CloseHandle drop f devhandle!
        DEV_NUM devcount get < [ t exit! ] when
        ! } while (DevCount < DEV_NUM);
   ] while
 
   ! SetupDiDestroyDeviceInfoList (DevInfo);
   devinfo SetupDiDestroyDeviceInfoList drop
   
   ! f hid devinfo<<
   ! devcount get
   detailvector
;




! Ge the name of device
:: hid-get-name ( device -- buf/f )
    ! HANDLE DevHandle;
    ! 0 :> devhandle
    ! WCHAR  wbuf[128];
    128 <byte-array> :> wbuf
    f :> flag!
    ! int    i;
    ! BOOL   ok;


    ! if (DevDetailData[num] == NULL) return (FALSE);
    device hid-device-info?
    [
    ! /* Create File for Device Read/Write */
    ! DevHandle = CreateFile(DevDetailData[num]->DevicePath, 
    !                 0, 
    !                 FILE_SHARE_READ | FILE_SHARE_WRITE, 
    !                 (LPSECURITY_ATTRIBUTES)NULL,
    !                 OPEN_EXISTING, 
    !                 0, 
    !                 NULL
    !                 );
        device path>>
        0
        flags{ FILE_SHARE_READ FILE_SHARE_WRITE }
        f
        OPEN_EXISTING
        0
        f
        CreateFile :> devhandle
        
        ! if (DevHandle == INVALID_HANDLE_VALUE) return (FALSE);
        devhandle INVALID_HANDLE_VALUE = not
        [
            ! Get the product string
            devhandle
            wbuf wbuf byte-length
            HidD_GetProductString 0 =
            [ f flag! ]
            [ wbuf native-string-encoding alien>string flag! ] if
            devhandle CloseHandle drop
        ] when
    ] when
    flag
;



! /*
!  *  HID Get Input Report Size
!  *    Parameters:      num:   Device number
!  *    Return Value:    Number of bytes for Input Report
!  */
! int HID_GetInputReportSize(int num) {

: hid-get-input-report-size ( hds -- n/f )
    dup
    [ inputreportsz>> ]
    [ drop f ] if
        

 ! return (DevInputReportSz[num]);
! }
;


! /*
!  *  HID Get Output Report Size
!  *    Parameters:      num:   Device number
!  *    Return Value:    Number of bytes for Output Report
!  */
! int HID_GetOutputReportSize(int num) {
: hid-get-output-report-size ( hds -- n/f )
    dup
    [ outputreportsz>> ]
    [ drop f ] if
 
!  return (DevOutputReportSz[num]);
! }
;

! /*
!  *  HID Get Feature Report Size
!  *    Parameters:      num:   Device number
!  *    Return Value:    Number of bytes for Output Report
!  */
! int HID_GetFeatureReportSize(int num) {
: hid-get-feature_report-size ( hds -- n/f )
    dup
    [ featurereportsz>> ]
    [ drop f ] if
    
!  return (DevFeatureReportSz[num]);
! }
;


! /*
!  *  HID Open
!  *    Parameters:      num:   Device number
!  *    Return Value:    TRUE - Success, FALSE - Error
!  */
! BOOL HID_Open(int num) {
:: hid-open ( hds -- ? )
    f :> retval!
    ! INVALID_HANDLE_VALUE :> devhandle  

    ! if (DevDetailData[num] == NULL) return (FALSE);
    hds
    [
        ! /* Create File for Device Read/Write */
        ! DevHandle = CreateFile(DevDetailData[num]->DevicePath, 
        !         GENERIC_READ | GENERIC_WRITE, 
        !         FILE_SHARE_READ | FILE_SHARE_WRITE, 
        !         (LPSECURITY_ATTRIBUTES)NULL,
        !         OPEN_EXISTING, 
        !         FILE_FLAG_OVERLAPPED, 
        !         NULL
        !         );
        hds devdetail>> DevicePath>>
        flags{ GENERIC_READ GENERIC_WRITE }
        flags{ FILE_SHARE_READ FILE_SHARE_WRITE }
        f
        OPEN_EXISTING
        FILE_FLAG_OVERLAPPED
        f
        CreateFile
        hds devhandle<<
        ! if (DevHandle == INVALID_HANDLE_VALUE) return (FALSE);
        hds devhandle>> INVALID_HANDLE_VALUE = not
        [
            ! DevSelected = num;
            
            ! memset(&DevOverlapped, 0, sizeof(OVERLAPPED));
            OVERLAPPED <struct> hds devoverlapped<<
            hds inputreportsz>> <byte-array> hds inputreport<<
            hds outputreportsz>> <byte-array> hds outputreport<<
            hds featurereportsz>> <byte-array> hds featurereport<<
            t retval!
        ] when
    ] when
    retval
;


! /*
!  *  HID Close
!  *    Parameters:      None
!  *    Return Value:    None
!  */
! void HID_Close() {
:: hid-close ( hds -- ? )
    f :> retval!
    ! DevSelected = -1;
    hds
    [
        hds devhandle>>
        [
            ! CancelIo(DevHandle);
            hds devhandle>>
            CancelIo 1 =
            [
                ! DevReadPending  = FALSE;
                f hds devreadpending<<
                ! if (DevHandle != INVALID_HANDLE_VALUE) {
                hds devhandle>> INVALID_HANDLE_VALUE = not
                [
                    ! CloseHandle(DevHandle);
                    hds devhandle>>
                    CloseHandle drop
                    f hds devhandle<<
                    ! DevHandle = INVALID_HANDLE_VALUE;
                    t retval!
                ] when
            ] when
        ] when
    ! }
    ] when
    retval
;

! /*
!  *  HID Read
!  *    Parameters:      buf:   Pointer to buffer that receives data
!  *                     sz:    Number of bytes to read
!  *                     cnt:   Pointer to number of bytes read
!  *    Return Value:    TRUE - Success, FALSE - Error
!  */
! BOOL HID_Read(BYTE *buf, DWORD sz, DWORD *cnt) {
:: hid-read ( hds cnt -- ? )
    f :> retval!
    !   int last_err;

    ! /* Read from Device */
    ! if (!DevReadPending) {
    hds devreadpending>> not
    [
        t hds devreadpending<<
        !   if (ReadFile(DevHandle, buf, sz, cnt, &DevOverlapped)) {
        hds devhandle>>
        hds inputreport>>
        hds inputreport>> byte-length
        cnt
        hds devoverlapped>>
        ReadFile 0 = not
        [
    !     return (TRUE);
            t retval!
            f hds devreadpending<<
    !   }
        ] when
    !   DevReadPending = TRUE;
    ]
    ! }
    ! else {
    [
        !   if (GetOverlappedResult(DevHandle, &DevOverlapped, cnt, FALSE)) {
        hds devhandle>>
        hds devoverlapped>>
        cnt
        0
        GetOverlappedResult 1 =
        [
        !      DevReadPending  = FALSE;
            f hds devreadpending<<
    !      return (TRUE);
           t retval!
        ] when
    !   }
    ! }
    ] if

    retval not
    [
        ! last_err = GetLastError();
        GetLastError
        :> last_error

        last_error .
        last_error ERROR_IO_INCOMPLETE = not
        last_error ERROR_IO_PENDING = not
        and
    
        ! if ((last_err != ERROR_IO_INCOMPLETE) && (last_err != ERROR_IO_PENDING)) {
        [
            !   DevReadPending  = FALSE;
            f hds devreadpending<<
            !   CancelIo(DevHandle);
            hds devhandle>>
            CancelIo drop
            f retval!
            !   return (FALSE);
            ! }
            ]
        [ t retval! ] if
    
        ! return (TRUE);
        ! }
    ] when
    retval
;

! /*
!  *  HID Write
!  *    Parameters:      buf:   Pointer to buffer with data to write
!  *                     sz:    Number of bytes to write
!  *                     cnt:   Pointer to number of bytes written
!  *    Return Value:    TRUE - Success, FALSE - Error
!  */
! BOOL HID_Write(BYTE *buf, DWORD sz, DWORD *cnt) {
:: hid-write ( hds cnt -- ? )
    f :> retval!
    ! /* Write to Device */
    ! if (WriteFile(DevHandle, buf, sz, cnt, &DevOverlapped))
    hds devhandle>>
    hds outputreport>>
    hds outputreport>> byte-length
    cnt
    hds devoverlapped>>
    WriteFile 1 =
    [
    !   return (TRUE);
        t retval!
    ]
    [
        ! if (GetLastError() == ERROR_IO_PENDING)
        GetLastError ERROR_IO_PENDING =
        [ t retval! ]
        !   return (TRUE);
        [
            ! CancelIo(DevHandle);
            hds devhandle>>
            CancelIo drop
            f retval!
        ] if
    ! return (FALSE);
! }
    ] if
    retval
;


! /*
!  *  HID SetFeature
!  *    Parameters:      buf:   Pointer to buffer with report to send
!  *                     sz:    Size of feature report to send
!  *    Return Value:    TRUE - Success, FALSE - Error
!  */
! BOOL HID_SetFeature(BYTE *buf, DWORD sz) {
:: hid-set-feature ( hds buf -- ? )
    f :> retval!
    ! return (HidD_SetFeature(DevHandle, buf, sz));
    hds
    [
        hds devhandle>>
        buf
        buf byte-length
        HidD_SetFeature
        retval!
    ]
    retval
;

