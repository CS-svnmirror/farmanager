﻿m4_include(`farversion.m4')m4_dnl
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!-- Copyright © 1996-2000 Eugene Roshal, Copyright © COPYRIGHTYEARS FAR Group -->
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
<assemblyIdentity
	version="MAJOR.MINOR.0.BUILD"
	processorArchitecture="*"
	name="Far Manager"
	type="win32"
/>
<description>File and archive manager</description>
<dependency>
	<dependentAssembly>
		<assemblyIdentity
			type="win32"
			name="Microsoft.Windows.Common-Controls"
			version="6.0.0.0"
			processorArchitecture="*"
			publicKeyToken="6595b64144ccf1df"
			language="*"
		/>
	</dependentAssembly>
</dependency>
<!-- Identify the application security requirements -->
<trustInfo xmlns="urn:schemas-microsoft-com:asm.v2">
	<security>
		<requestedPrivileges>
			<requestedExecutionLevel
				level="asInvoker"
				uiAccess="false"
			/>
		</requestedPrivileges>
	</security>
</trustInfo>
</assembly>