<?php

/**
 * @file
 * Contains views hooks.
 */

use Drupal\Core\Entity\ContentEntityTypeInterface;
use Drupal\Core\Field\FieldDefinitionInterface;

/**
 * Implements hook_views_data_alter().
 */
function entityreference_filter_custom_views_data_alter(array &$data) {
  static $status;
  // Apply to entity reference fields, nid, uid, tid and commerce variations.
  foreach ($data as $table_name => $table_info) {
    $fieldDefinitions = [];
    $entity_type = NULL;
    if (!empty($table_info['table']['entity type'])) {
      $entity_type = \Drupal::entityTypeManager()->getDefinition($table_info['table']['entity type']);
      if ($entity_type) {
        $fieldDefinitions = \Drupal::service('entity_field.manager')->getBaseFieldDefinitions($entity_type->id());
      }
    }
    if (!$fieldDefinitions) {
      continue;
    }

    foreach ($table_info as $field_name => $field_info) {
      $base_table = NULL;
      $target_entity_type = NULL;

      if (!isset($fieldDefinitions[$field_name])) {
        continue;
      }
      /** @var FieldDefinitionInterface $definition */
      $fieldDefinition = $fieldDefinitions[$field_name];

      if($fieldDefinition->getType() === 'entity_reference') {
        $settings = $fieldDefinition->getItemDefinition()->getSettings();
        $target_entity_type = $settings['target_type'];
      }

      if (isset($data[$table_name][$field_name . '_entityreference_filter'])) {
        continue;
      }

      if (!$target_entity_type) {
        continue;
      }

      /** @var \Drupal\Core\Entity\EntityTypeInterface $entity_type_info */
      $target_entity_type_info = Drupal::entityTypeManager()->getDefinition($target_entity_type);

      // Content entities.
      if ($target_entity_type_info instanceof ContentEntityTypeInterface) {
        $base_table = $target_entity_type_info->getDataTable();
        if (!$base_table) {
          $base_table = $target_entity_type_info->getBaseTable();
        }
      }

      // @todo Config entities support.
      if (empty($base_table)) {
        continue;
      }

      if ($base_table && !empty($field_info['filter']['id']) &&
        $field_info['filter']['id'] !== 'entityreference_filter_view_result') {

        $filter = $field_info;

        // Filter title.
        /** @var \Drupal\Core\StringTranslation\TranslatableMarkup $title */
        $title = !empty($field_info['filter']['title']) ?
          $field_info['filter']['title'] : $field_info['title'];

        $title .= '';
        $filter['title'] = t('@title (entityreference filter)', ['@title' => $title]);

        // Filter title short.
        /** @var \Drupal\Core\StringTranslation\TranslatableMarkup $title */
        $title_short = !empty($field_info['filter']['title short']) ?
          $field_info['filter']['title short'] : $field_info['title'];

        $title_short .= '';
        $filter['title short'] = t('@title (ef)', ['@title' => $title_short]);

        $filter['filter']['id'] = 'entityreference_filter_view_result';
        // @todo do we need it ?
        $filter['filter']['proxy'] = !empty($field_info['filter']['handler']) ? $field_info['filter']['handler'] : NULL;

        $filter['filter']['field'] = $field_name;
        $filter['filter']['filter_base_table'] = $base_table;

        // Adds only filter field.
        unset($filter['filter']['title'], $filter['argument'], $filter['field'], $filter['relationship'], $filter['sort']);

        $data[$table_name][$field_name . '_entityreference_filter'] = $filter;
      }
    }
  }
}
