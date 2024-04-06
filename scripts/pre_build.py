import datetime
import os
import re

current_time = datetime.datetime.now().strftime('v_%d%m%Y.%H%M%S')
version_h_path = os.path.join('src', 'defines', 'version.h')

if not os.path.exists(version_h_path):
    with open(version_h_path, 'w') as f:
        f.write('#ifndef VERSION_H\n')
        f.write('#define VERSION_H\n')
        f.write(f'#define BUILD_TIME "{current_time}"\n')
        f.write(f'#define FIRMWARE_VERSION "v2024.04_0.1"\n')
        f.write('#endif\n')
else:
    with open(version_h_path, 'r') as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if line.startswith('#define BUILD_TIME'):
            lines[i] = f'#define BUILD_TIME "{current_time}"\n'
        elif line.startswith('#define FIRMWARE_VERSION'):
            version_match = re.match(r'#define FIRMWARE_VERSION "v(\d{4})\.(\d{2})_(\d+)\.(\d+)"', line)
            if version_match:
                year = version_match.group(1)
                month = version_match.group(2)
                major = version_match.group(3)
                minor = int(version_match.group(4))
                minor += 1  # Incrementing minor version
                lines[i] = f'#define FIRMWARE_VERSION "v{year}.{month}_{major}.{minor}"\n'
            else:
                print("Error: Invalid format for FIRMWARE_VERSION")
                exit(1)

    with open(version_h_path, 'w') as f:
        f.writelines(lines)