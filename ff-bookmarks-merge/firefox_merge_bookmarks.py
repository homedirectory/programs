import json

FOLDER_TYPE = "text/x-moz-place-container"
BOOKMARK_TYPE = "text/x-moz-place"

ROOT_ROOT = "placesRoot"

# merge books1 with books2 with the result being stored in the one that is bigger
def merge_bookmarks(books1, books2):
    tree1 = create_tree(books1)
    tree2 = create_tree(books2)
    print("Tree 1 size: {}".format(tree1.size()))
    print("Tree 2 size: {}".format(tree2.size()))
    merged = {}
    if (tree1.size() > tree2.size()):
        merged = merge(tree1, tree2)
    else:
        merged = merge(tree2, tree1)

    print("Merged size: {}".format(merged.size()))

    return merged.to_json()

# merges t2 into t1
def merge(t1, t2):
    for child in t2.children:
        t1_node = find_node(t1, child)
        if not t1_node:
            debug_print("Adding {} to {}.".format(child, t1))
            #print(child.__dict__)
            #print()
            #print(t1.__dict__)
            t1.children.append(child)
        else:
            if t1_node.is_folder:
                debug_print("Merging {} into {}.".format(child, t1))
                merge(t1_node, child)
            else:
                if child.more_recent(t1_node):
                    debug_print("Updating {}.".format(t1_node))
                    t1_node.update(child)

    return t1


class BookmarkNode:

    def __init__(self, dict_data):
        for key in dict_data:
            self.__dict__[key] = dict_data[key]
        self.is_folder = (self.type == FOLDER_TYPE)
        self.children = []

    def size(self):
        if self.is_folder:
            count = int(not self.is_root_folder())
            return count + sum([c.size() for c in self.children])
        return 1

    def is_root_folder(self):
        if not self.is_folder:
            return False
        return hasattr(self, "root")

    def is_tree_root(self):
        if not self.is_root_folder():
            return False
        return self.root == ROOT_ROOT

    def equals(self, node):
        if self.type != node.type:
            return False
        elif self.is_folder:
            return self.parentNode == node.parentNode and self.title == node.title
        else:
            return self.parentNode == node.parentNode and self.uri == node.uri

    def more_recent(self, node):
        return self.lastModified > node.lastModified

    def update(self, node):
        for key in node.__dict__:
            self.__dict__[key] = node.__dict__[key]

    def to_json(self):
        if not self.is_root_folder():
            self.__dict__.pop("guid")
        self.__dict__.pop("parentNode")
        self.__dict__.pop("is_folder")

        children = self.__dict__.pop("children", [])
        children_json = []
        for child in children:
            children_json.append(child.to_json())
        if len(children_json) > 0:
            self.__dict__["children"] = children_json
        return self.__dict__

    def __str__(self):
        if self.is_root_folder():
            return "\"{}\"".format(self.root)
        elif self.is_folder:
            return "Folder \"{}\"".format(self.title)
        else:
            return "{} ({})".format(self.title, self.uri)


def create_tree(dict_data):
    def _tree(data, parent):
        children = data.pop("children", [])
        data["parentNode"] = parent
        node = BookmarkNode(data)
        for child in children:
            node.children.append(_tree(child, parent))
        return node

    root_folders = dict_data.pop("children", [])
    dict_data["parentNode"] = None
    root = BookmarkNode(dict_data)
    # a special loop for root folders to set their parent to tree root
    for rf in root_folders:
        children = rf.pop("children", [])
        rf["parentNode"] = root.root
        node = BookmarkNode(rf)
        root.children.append(node)
        # children of a root folder
        for child in children:
            node.children.append(_tree(child, node.root))

    return root

def find_node(tree, node):
    if not tree.is_folder:
        raise Exception("Trying to find a node inside a regular bookmark node instead of a tree.")
    elif node.is_root_folder():
        lst = list(filter(lambda x: x.is_root_folder() and x.root == node.root, tree.children))
        if len(lst) == 0:
            raise Exception("Root folder {} not found in tree.".format(node.root, tree))
        return lst[0]
    else:
        if tree.is_tree_root():
            # look into root folder which is parent to node
            root_folders = list(filter(lambda x: x.root == node.parentNode, tree.children))
            if len(root_folders == 0):
                raise Exception("parentNode {} not found in tree.".format(node.parentNode))
            return find_node(root_folders[0], node)

        elif tree.is_root_folder():
            if tree.root != node.parentNode:
                return None
        # if tree and node are in different root folders - skip
        elif tree.parentNode != node.parentNode:
            return None

        # now we know that tree must be node's parent
        # depth-first search
        for child in tree.children:
            if child.equals(node):
                return child
            elif child.is_folder:
                sub = find_node(child, node)
                if sub:
                    return sub

        return None

def debug_print(msg):
    print("[DEBUG] {}".format(msg))


if __name__ == "__main__":
    import sys

    USAGE = "Usage: {} [FILE1.json] [FILE2.json] [OUT.json]".format(sys.argv[0])

    try:
        fname1, fname2, fname_out = sys.argv[1:4]
    except Exception as e:
        print(USAGE)
        sys.exit(1)

    with open(fname1, "rb") as f:
        data1 = json.loads(f.read().decode('utf_8'))
    with open(fname2, "rb") as f:
        data2 = json.loads(f.read().decode('utf_8'))

    merged = merge_bookmarks(data1, data2)

    with open(fname_out, 'w') as f:
        json.dump(merged, f, ensure_ascii=False)
