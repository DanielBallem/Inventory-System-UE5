# Inventory-System-UE5
c++ and BP implementation of a full-feature inventory system

Features: 
1. BP_InventoryComponent which extends a c++ InventoryComponent containing all inventory-related functionality.
2. Depositing items into stacks of varying sizes. Stack size and type dictated by data tables.
3. Swapping two stacks of different types.
4. Drag and drop UI. Makes calls to different methods in inventory component to facilitate changes.
5. Splitting stacks.
6. Fully supported interactions between inventories (containers, armour).
7. Restricting inventory slots to certain types (Armour)

## Showcase

Full support for all interactions within and between other inventory objects. 

### Drag and dropping:

<img src="https://github.com/DanielBallem/Inventory-System-UE5/blob/main/githubimages/draganddrop2.gif" alt="Databay showcase gif" title="Databay showcase gif" width="500"/>

### Picking up items:
Items are stored in slots, and their max stack size is dictated by a data table.
Items slots of the same item are filled before empty slots are utilized. 

![image](https://github.com/DanielBallem/Inventory-System-UE5/assets/33844493/71fcc57e-5799-4654-b4e6-c1d40fb135bf)

<img src="https://github.com/DanielBallem/Inventory-System-UE5/blob/main/githubimages/pickingupitems.gif" alt="Databay showcase gif" title="Databay showcase gif" width="500"/>

### Restricting slots:
By setting an item type and setting a slot restriction, items cannot fill certain slots unless they are of the type. Swapping, splitting and dropping are safe operations to make and won't break the slot restrictions.

Here, the UI shows slot restrictions by an icon. The right items can be dropped to fill them in.

<img src="https://github.com/DanielBallem/Inventory-System-UE5/blob/main/githubimages/restrictiveslots.gif" alt="Databay showcase gif" title="Databay showcase gif" width="500"/>

### Stack splitting:
Holding Shift and dragging off from an item will split the stack if more than one exists. If it's an odd number, the stack's origin will retain the extra item. It can be dropped into an empty slot or added onto a stack of the same item with available space.

<img src="https://github.com/DanielBallem/Inventory-System-UE5/blob/main/githubimages/splittingstacks.gif" alt="Databay showcase gif" title="Databay showcase gif" width="500"/>
