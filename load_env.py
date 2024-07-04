from os.path import isfile
Import("env")

try:
    assert isfile(".env")
    f = open('.env', 'r')
    lines = f.readlines()
    envs = []
    for line in lines:
        if line in ['\n', '\r\n', '\r']:
            continue

        if "#" in line:
            continue

        flag = line.strip()
        flag_arr = flag.split("=")
        
        try:
            key = flag_arr[0]
            value = flag_arr[1]

            if value.isdigit():
                newValue = int(value)
            else:
                newValue = float(value)
        except ValueError:
            newValue = env.StringifyMacro(value.replace("\"", ""))
        finally:
            envs.append((key, newValue))

    env.Append(CPPDEFINES=envs)
    print("ENV: Loaded variabel environment")
except AssertionError as e:
    print(f"File .env tidak ada")
