<?php

/**
 * @file
 * Default implementations of the entity_state_block hooks.
 */

// @todo: more implementations: domains, revisions, last node update by, translations state, etc.

if (!function_exists('node_entity_state_block_info')) {

  /**
   * Implements hook_entity_state_block_info().
   */
  function node_entity_state_block_info() {
    return array(
      'node' => array(
        'node_status' => array(
          'title' => t('Content status'),
          'description' => t('Whether a node is published or not.'),
          'weight' => -100,
          'enabled' => TRUE,
        ),
        'node_author' => array(
          'title' => t('Content author'),
          'description' => t('The value of the "Authored by" content property.'),
        ),
      ),
    );
  }

}
if (!function_exists('node_entity_state_block_view')) {

  /**
   * Implements hook_entity_state_block_view().
   */
  function node_entity_state_block_view($entity_type, $entity, $enabled_sections) {
    $return = array();
    if ($entity_type == 'node') {
      if (in_array('node_status', $enabled_sections)) {
        $return['node_status'] = array(
          'title' => t('Status'),
          'value' => $entity->status == NODE_PUBLISHED ? t('Published') : t('Unpublished'),
        );
      }
      if (in_array('node_author', $enabled_sections)) {
        $return['node_author'] = array(
          'title' => t('Authored by'),
          'value' => theme('username', array('account' => $entity)),
        );
      }
    }
    return $return;
  }
}
