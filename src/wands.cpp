// Copyright (c) 1989-2008 James E. Wilson, Robert A. Koeneke, David J. Grabiner
//
// Umoria is free software released under a GPL v2 license and comes with
// ABSOLUTELY NO WARRANTY. See https://www.gnu.org/licenses/gpl-2.0.html
// for further details.

// Wand code

#include "headers.h"
#include "externs.h"

static bool discharge_wand(Inventory_t *wand, int dir) {
    bool identified = false;

    uint32_t flags = wand->flags;

    (wand->p1)--; // decrement "use" variable

    while (flags != 0) {
        int kind = bit_pos(&flags) + 1;
        int row = char_row;
        int col = char_col;

        // Wand types
        switch (kind) {
            case 1:
                msg_print("A line of blue shimmering light appears.");
                light_line(char_col, char_row, dir);
                identified = true;
                break;
            case 2:
                fire_bolt(row, col, dir, damroll(4, 8), GF_LIGHTNING, spell_names[8]);
                identified = true;
                break;
            case 3:
                fire_bolt(row, col, dir, damroll(6, 8), GF_FROST, spell_names[14]);
                identified = true;
                break;
            case 4:
                fire_bolt(row, col, dir, damroll(9, 8), GF_FIRE, spell_names[22]);
                identified = true;
                break;
            case 5:
                identified = wall_to_mud(row, col, dir);
                break;
            case 6:
                identified = poly_monster(row, col, dir);
                break;
            case 7:
                identified = hp_monster(row, col, dir, -damroll(4, 6));
                break;
            case 8:
                identified = speed_monster(row, col, dir, 1);
                break;
            case 9:
                identified = speed_monster(row, col, dir, -1);
                break;
            case 10:
                identified = confuse_monster(row, col, dir);
                break;
            case 11:
                identified = sleep_monster(row, col, dir);
                break;
            case 12:
                identified = drain_life(row, col, dir);
                break;
            case 13:
                identified = td_destroy2(row, col, dir);
                break;
            case 14:
                fire_bolt(row, col, dir, damroll(2, 6), GF_MAGIC_MISSILE, spell_names[0]);
                identified = true;
                break;
            case 15:
                identified = build_wall(row, col, dir);
                break;
            case 16:
                identified = clone_monster(row, col, dir);
                break;
            case 17:
                identified = teleport_monster(row, col, dir);
                break;
            case 18:
                identified = disarm_all(row, col, dir);
                break;
            case 19:
                fire_ball(row, col, dir, 32, GF_LIGHTNING, "Lightning Ball");
                identified = true;
                break;
            case 20:
                fire_ball(row, col, dir, 48, GF_FROST, "Cold Ball");
                identified = true;
                break;
            case 21:
                fire_ball(row, col, dir, 72, GF_FIRE, spell_names[28]);
                identified = true;
                break;
            case 22:
                fire_ball(row, col, dir, 12, GF_POISON_GAS, spell_names[6]);
                identified = true;
                break;
            case 23:
                fire_ball(row, col, dir, 60, GF_ACID, "Acid Ball");
                identified = true;
                break;
            case 24:
                flags = (uint32_t) (1L << (randint(23) - 1));
                break;
            default:
                msg_print("Internal error in wands()");
                break;
        }
    }

    return identified;
}

// Wands for the aiming.
void aim() {
    player_free_turn = true;

    if (inventory_count == 0) {
        msg_print("But you are not carrying anything.");
        return;
    }

    int j, k;
    if (!find_range(TV_WAND, TV_NEVER, &j, &k)) {
        msg_print("You are not carrying any wands.");
        return;
    }

    int item_id;
    if (!get_item(&item_id, "Aim which wand?", j, k, CNIL, CNIL)) {
        return;
    }

    player_free_turn = false;

    int dir;
    if (!get_dir(CNIL, &dir)) {
        return;
    }

    if (py.flags.confused > 0) {
        msg_print("You are confused.");
        dir = getRandomDirection();
    }

    Inventory_t *item = &inventory[item_id];

    int player_class_lev_adj = class_level_adj[py.misc.pclass][CLA_DEVICE] * py.misc.lev / 3;
    int chance = py.misc.save + stat_adj(A_INT) - (int) item->level + player_class_lev_adj;

    if (py.flags.confused > 0) {
        chance = chance / 2;
    }

    if (chance < USE_DEVICE && randint(USE_DEVICE - chance + 1) == 1) {
        chance = USE_DEVICE; // Give everyone a slight chance
    }

    if (chance <= 0) {
        chance = 1;
    }

    if (randint(chance) < USE_DEVICE) {
        msg_print("You failed to use the wand properly.");
        return;
    }

    if (item->p1 < 1) {
        msg_print("The wand has no charges left.");
        if (!spellItemIdentified(item)) {
            add_inscribe(item, ID_EMPTY);
        }
        return;
    }

    bool ident = discharge_wand(item, dir);

    if (ident) {
        if (!itemSetColorlessAsIdentifed(item)) {
            // round half-way case up
            py.misc.exp += (item->level + (py.misc.lev >> 1)) / py.misc.lev;
            prt_experience();

            identify(&item_id);
        }
    } else if (!itemSetColorlessAsIdentifed(item)) {
        itemSetAsTried(item);
    }

    desc_charges(item_id);
}
