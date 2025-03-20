#!/usr/bin/env pwsh

Remove-Item -Recurse -Force build
New-Item -ItemType Directory -Force -Path build
Set-Location -Path build
$output_file = "djinni_example"
if ($IsWindows) {
    $output_file += ".exe"
    cmake -S .. -B . -G "Visual Studio 17 2022"
    MsBuild.exe djinni.sln /t:djinni_example /p:Platform="x64" /p:Configuration=Release
    Copy-Item src\Release\$output_file -Destination ..\$output_file
    Set-Location -Path ..
    Remove-Item -Recurse -Force build
    New-Item -ItemType Directory -Path djinni
    Copy-Item src\djinni.h -Destination djinni\djinni.h
    Copy-Item src\djinni -Destination djinni -Recurse
    Compress-Archive -Path djinni -CompressionLevel Optimal -DestinationPath djinni.zip
    Copy-Item "src\Dumas-1.set" -Destination "..\Dumas-1.set"
    Remove-Item -Recurse -Force djinni
    Write-Host @"
* * * * * * * * * *

Djinni has successfully built.  You have two new files in this directory:

* djinni_example.exe, a simple test application you can run
* Dumas-1.set, a Dumas-formatted file containing a set of cities a
  traveling salesman must efficiently visit, and
* djinni.zip, containing all you need to use Djinni in your own code.

To use Djinni in your own code:

* uncompress djinni.zip somewhere
* tell your compiler to add that directory to your include path
  (/I\where\I\put\it works well on Microsoft's compiler)
* tell your compiler to enable standard C++ exception handling
  (/EHsc on Microsoft's)
* tell your compiler to optimize the code for performance
  (/O2 on Microsoft's)
* tell your compiler to use the C++23 standard (/std:c++latest on
  Microsoft's)
* tell your compiler what to call the output (/Fe on Microsoft's)
* add an '#include <djinni.h>' to your C++ sources

For instance, to compile src/example.cc with Microsoft's compiler,
you could do it from the command line with:

* cl.exe /EHsc /IC:\djinni /std:c++latest /O2 /Fedjinni_example.exe src/example.cc

... and you're off to the races.  You can also type '.\djinni_example.exe'
in this directory to see it in action, study the file 'Dumas-1.set' to
see the problem it's approximately solving, or look at 'src\example.cc'
to see the source code of the example.

Happy hacking!

    -- Rob
"@
} else {
    make -j8
    if ("" -ne (groups | Select-String -Raw -Pattern "wheel")) {
        cmake -S .. -B . -D CMAKE_BUILD_TYPE=Release
        make -j8
        $install_path="/usr/local"
        Write-Host @"
Installing systemwide...
"@
        sudo make install
    } else {
        cmake -S .. -B . -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=${env:HOME}
        make -j8
        Write-Host @"
Installing to your home directory...
"@
        $install_path=$env:HOME
        make install
    }

    Copy-Item src/$output_file ../$output_file
    strip ../$output_file
    Set-Location -Path ..
    Remove-Item -Recurse -Force build
    Copy-Item "src\Dumas-1.set" -Destination ".\Dumas-1.set"
    Write-Host @"
* * * * * * * * * *

Djinni has been installed to $install_path/include

To use Djinni in your own code:

* tell your compiler to add ${install_path/include} to your include path
  (-I${install_path}/include works for most compilers)
* tell your compiler to optimize the code for performance
  (-O2 works for most)
* tell your compiler to use the C++23 standard (-std=c++23 on most)
* tell your compiler what to call the output (-o on most)
* add an '#include <djinni.h>' to your C++ sources

For instance, to compile src/example.cc with your choice of the
Intel icpx, LLVM clang++, or GNU C++ compilers, you could enter:

* icpx src/example.cc -std=c++23 -I${install_path}/include -O2 -o djinni_example
* clang++ src/example.cc -std=c++23 -I${install_path}/include -O2 -o djinni_example
* g++ src/example.cc -std=c++23 -I${install_path}/include -O2 -o djinni_example

... and you're off to the races.  You can also type './djinni_example'
in this directory to see it in action, study the file 'Dumas-1.set' to
see the problem it's approximately solving, or look at 'src/example.cc'
to see the source code of the example.

Happy hacking!

    -- Rob
"@
}
