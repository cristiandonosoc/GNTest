import sys

def main(path):
    open(path, "w").close()

if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
