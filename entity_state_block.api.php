<?php

/**
 * Defines entity state sections grouped by entity types.
 *
 * @return array
 *   See code for array structure.
 */
function hook_entity_state_block_info() {
  return array(
    'node' => array( // Entity type.
      'node_status' => array( // Section machine name.
        'title' => t('Content status'), // Title showed on the configuration page.
        'description' => t('Whether a node is published or not.'), // Description for the configuration page.
        // The following properties are optional and can be overridden by user.
        'weight' => -100, // Default: 0
        'enabled' => TRUE, // Default: FALSE
      ),
    ),
  );
}

/**
 * Returns entity state sections.
 *
 * @param string $entity_type
 * @param object $entity
 * @param array $enabled_sections
 *   A numeric array with section machine names. Contains only enabled sections.
 *
 * @return array
 *   See code for array structure.
 *   The returned array should only contain enabled sections.
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
  }
  return $return;
}
