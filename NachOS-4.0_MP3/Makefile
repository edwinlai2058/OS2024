image = dasbd72/nachos:dev-v2.2

.Phony: pull
pull:
	@echo "Pulling image from Docker Hub..."
	@docker pull $(image)

.Phony: push
push:
	@echo "Pushing image to Docker Hub..."
	@docker push $(image)

.Phony: build
build:
	@echo "Building image..."
	@docker build -t $(image) .

MOUNT_DIR_RO = code/filesys code/lib code/machine code/network code/threads code/userprog code/build.linux/Makefile code/build.linux/Makefile.dep # Read-only directories
MOUNT_DIR_RW = code/test # Read-write directories
MOUNT_OPT = $(foreach dir,$(MOUNT_DIR_RO),-v $(CURDIR)/$(dir):/nachos/$(dir):ro) $(foreach dir,$(MOUNT_DIR_RW),-v $(CURDIR)/$(dir):/nachos/$(dir))

.Phony: run
run:
	@echo "Running container..."
	@docker run --rm -it --platform=linux/amd64 $(MOUNT_OPT) $(image)
