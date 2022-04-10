import argparse
import sys
from logging import DEBUG as logging_DEBUG
from logging import INFO as logging_INFO
from logging import FileHandler, Formatter, StreamHandler, getLogger

# stdout Handler
stdout_handler = StreamHandler(stream=sys.stdout)
stdout_handler.setLevel(logging_DEBUG)  # DEBUGまで出すが上の階層次第で出さないこともある
stdout_handler.addFilter(
    lambda record: record.levelno <= logging_INFO
)  # warning, error, criticalを出さない

stdout_handler.setFormatter(
    Formatter(
        "%(asctime)s [%(levelname)s] - %(filename)s - %(funcName)s() --- %(message)s"
    )
)

# file Handler
file_handler = FileHandler("out.log")
file_handler.setLevel(logging_DEBUG)
file_handler.setFormatter(
    Formatter(
        "%(asctime)s [%(levelname)s] - %(filename)s - %(funcName)s() --- %(message)s"
    )
)


# Logger
logger = getLogger(__name__)

logger.addHandler(stdout_handler)
logger.addHandler(file_handler)

logger.setLevel(logging_INFO)  # DEBUGは出さない


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--debug", action="store_true")
    args = parser.parse_args()

    if args.debug:
        logger.setLevel(logging_DEBUG)  # DEBUGも出力するようにする

    logger.debug("This is a debug log.")

    logger.info("This is a info log.")

    logger.warning("This is a warning log.")

    logger.error("This is a error log.")

    logger.critical("This is a critical log.")


if __name__ == "__main__":
    main()
